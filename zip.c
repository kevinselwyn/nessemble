#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "third-party/udeflate/deflate.h"

#define ZIP_BUF_SIZE   512 * 1024
#define TAR_BLOCK_SIZE 512

#define IN_SIZE  (1 << 12)
#define OUT_SIZE (1 << 16)

static char input[IN_SIZE], output[OUT_SIZE];

static unsigned int i_in, i_out;

int udeflate_read_bits(int n_bits) {
    int next = 0, i = 0, ret = 0;

    if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)IN_SIZE) {
		return -EINVAL;
    }

	for (i = 0; i < n_bits; i++, i_in++) {
		next = (int)((unsigned int)input[i_in >> 3] >> (i_in & 0x7)) & 1;
		ret |= (unsigned int)next << (unsigned int)i;
	}

	return ret;
}

int udeflate_read_huffman_bits(int n_bits) {
    int next = 0, i = 0, ret = 0;

	if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)IN_SIZE) {
		return -EINVAL;
    }

	for (i = 0; i < n_bits; i++, i_in++) {
		next = (int)((unsigned int)input[i_in >> 3] >> (i_in & 0x7)) & 1;
		ret = (int)((unsigned int)ret << 1) | next;
	}

	return ret;
}

int udeflate_peek_huffman_bits(int n_bits) {
    int next = 0, i = 0, tmp_i_in = 0, ret = 0;

	if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)IN_SIZE) {
		return -EINVAL;
    }

	tmp_i_in = (int)i_in;

	for (i = 0; i < n_bits; i++, tmp_i_in++) {
		next = (int)(((unsigned int)input[(unsigned int)tmp_i_in >> 3] >> (unsigned int)(tmp_i_in & 0x7)) & 1);
		ret = (int)((unsigned int)ret << 1) | next;
	}

	return ret;
}

int udeflate_read_next_byte() {
    int ret = 0;

	if ((((i_in + 7) & (~7)) >> 3) + 1 > (unsigned int)IN_SIZE) {
		return -EINVAL;
    }

	i_in = (i_in + 7) & (~7);

	ret = (int)input[i_in >> 3];

	i_in += 8;

	return ret;
}

int udeflate_write_byte(uint8_t data) {
	if ((i_out + 1) > (unsigned int)OUT_SIZE) {
		return -EOVERFLOW;
    }

	output[i_out++] = (char)data;

	return 0;
}

int udeflate_write_match(uint16_t len, uint16_t dist) {
    int i = 0;
    char *ptr = NULL;

	if ((i_out + (unsigned int)len) > (unsigned int)OUT_SIZE) {
		return -EOVERFLOW;
    }

	if ((int)(i_out - dist) < 0) {
		return -EINVAL;
    }

	ptr = &output[i_out - dist];

	for (i = 0; i < (int)len; i++) {
		output[i_out++] = *(ptr++);
    }

	return 0;
}


int udeflate_write_input_bytes(uint16_t len) {
	if ((i_out + (unsigned int)len) > (unsigned int)OUT_SIZE) {
		return -EOVERFLOW;
    }

	if (((i_in >> 3) + (unsigned int)len) > (unsigned int)IN_SIZE) {
		return -EINVAL;
    }

	memcpy(&output[i_out], &input[i_in >> 3], len);

	i_out += len;
	i_in += 8*len;

	return 0;
}

static unsigned int untar(char **data, size_t *data_length, char *tar, unsigned int tar_length, char *filename) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int size = 0, remaining = 0;
    char block[512], tar_filename[100];
    char *filedata = NULL;

    l = (unsigned int)tar_length;

    while (i < l) {
        memcpy(block, tar+i, TAR_BLOCK_SIZE);
        memcpy(tar_filename, block, 100);

        if (tar_filename[0] == '\0') {
            break;
        }

        i += TAR_BLOCK_SIZE;
        size = (unsigned int)oct2int(block+124);
        remaining = (((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE) - size;

        if (strcmp(tar_filename, filename) == 0) {
            filedata = (char *)malloc(sizeof(char) * (size + 1));

            if (!filedata) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            memcpy(filedata, tar+i, (size_t)size);

            *data = filedata;
            *data_length = (size_t)size;

            break;
        }

        i += size + remaining;
    }

cleanup:
    return rc;
}

unsigned int get_unzipped(char **data, size_t *data_length, char *filename, char *url) {
    unsigned int rc = RETURN_OK, http_code = 0, content_length = 0, index = 0;
    char *content = NULL;

    http_code = get_request(&content, &content_length, url, MIMETYPE_ZIP);

    switch (http_code) {
    case 503:
        error_program_log("Could not reach the registry");

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log("Library does not exist");

        rc = RETURN_EPERM;
        goto cleanup;
    case 200:
    default:
        break;
    }

    if (!content) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (content[index] != '\0') {
        index++;

        if (index >= content_length) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    }

    index++;

    memcpy(input, content+index, (size_t)(content_length - index));

    if (deflate() < 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = untar(&*data, &*data_length, output, i_out, filename)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    if (content) {
        nessemble_free(content);
    }

    return rc;
}
