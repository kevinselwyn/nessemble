#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "http.h"
#include "third-party/udeflate/deflate.h"

static char input[ZIP_INSIZE], output[ZIP_OUTSIZE];

static unsigned int i_in, i_out;

int udeflate_read_bits(int n_bits) {
    int next = 0, i = 0, ret = 0;

    if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)ZIP_INSIZE) {
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

	if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)ZIP_INSIZE) {
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

	if ((((i_in + n_bits + 7) & (~7)) >> 3) > (unsigned int)ZIP_INSIZE) {
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

	if ((((i_in + 7) & (~7)) >> 3) + 1 > (unsigned int)ZIP_INSIZE) {
		return -EINVAL;
    }

	i_in = (i_in + 7) & (~7);

	ret = (int)input[i_in >> 3];

	i_in += 8;

	return ret;
}

int udeflate_write_byte(uint8_t data) {
	if ((i_out + 1) > (unsigned int)ZIP_OUTSIZE) {
		return -EOVERFLOW;
    }

	output[i_out++] = (char)data;

	return 0;
}

int udeflate_write_match(uint16_t len, uint16_t dist) {
    int i = 0;
    char *ptr = NULL;

	if ((i_out + (unsigned int)len) > (unsigned int)ZIP_OUTSIZE) {
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
	if ((i_out + (unsigned int)len) > (unsigned int)ZIP_OUTSIZE) {
		return -EOVERFLOW;
    }

	if (((i_in >> 3) + (unsigned int)len) > (unsigned int)ZIP_INSIZE) {
		return -EINVAL;
    }

	memcpy(&output[i_out], &input[i_in >> 3], len);

	i_out += len;
	i_in += 8*len;

	return 0;
}

unsigned int untar_list(char ***filenames, size_t *filenames_count, char *tar, unsigned int tar_length) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int size = 0, remaining = 0;
    size_t output_filenames_count = 0;
    char block[512], tar_filename[100];
    char **output_filenames = NULL;

    output_filenames = (char **)nessemble_malloc(sizeof(char *) * 100);

    l = tar_length;

    while (i < l) {
        memcpy(block, tar+i, TAR_BLOCK_SIZE);
        memcpy(tar_filename, block, 100);

        if (tar_filename[0] == '\0') {
            break;
        }

        i += TAR_BLOCK_SIZE;
        size = (unsigned int)oct2int(block+124);
        remaining = (((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE) - size;

        output_filenames[output_filenames_count++] = nessemble_strdup(tar_filename);

        i += size + remaining;
    }

    *filenames = output_filenames;
    *filenames_count = output_filenames_count;

    return rc;
}

unsigned int untar(char **data, size_t *data_length, char *tar, unsigned int tar_length, char *filename) {
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

unsigned int get_ungzip(char **data, size_t *data_length, char *buffer, unsigned int buffer_length) {
    unsigned int rc = RETURN_OK;
    size_t data_output_length = 0;
    char *data_output = NULL;

    memcpy(input, buffer, (size_t)buffer_length);

    if (deflate() < 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data_output = (char *)nessemble_malloc(sizeof(char) * (i_out + 1));
    memcpy(data_output, output, i_out);
    data_output_length = (size_t)i_out;

cleanup:
    *data = data_output;
    *data_length = data_output_length;

    i_in = 0;
    i_out = 0;
    memset(input, '\0', ZIP_INSIZE);
    memset(output, '\0', ZIP_OUTSIZE);

    return rc;
}

unsigned int get_unzipped(char **data, size_t *data_length, char *url) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int index = 0, checksummed = FALSE;
    char *shasum = NULL;
    http_t request;

    http_init(&request);

    if ((rc = http_get(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    switch (request.status_code) {
    case 503:
        error_program_log(_("Could not reach the registry"));

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log(_("Package does not exist"));

        rc = RETURN_EPERM;
        goto cleanup;
    case 200:
    default:
        break;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    hash(&shasum, request.response_body, request.content_length);

    for (i = 0, l = request.response_header_count; i < l; i++) {
        if (strcmp(request.response_headers[i].key, "X-Integrity") == 0) {
            if (strcmp(request.response_headers[i].val, shasum) == 0) {
                checksummed = TRUE;
            }
        }
    }

    if (checksummed == FALSE) {
        error_program_log(_("Invalid shasum (%s)"), shasum);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (request.response_body[index] != '\0') {
        index++;

        if (index >= request.content_length) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    }

    index++;

    if ((rc = get_ungzip(&*data, &*data_length, request.response_body+index, request.content_length - index)) != RETURN_OK) {
        goto cleanup;
    }

    http_release(&request);

cleanup:
    nessemble_free(shasum);

    return rc;
}
