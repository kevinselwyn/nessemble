#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "third-party/udeflate/deflate.h"

static char input[ZIP_INSIZE], output[ZIP_OUTSIZE];

static unsigned int i_in, i_out;

char *cache_url, *cache_content;
unsigned int cache_content_length = 0;
struct http_header cache_response_headers = { 0, { }, { } };

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

unsigned int get_unzipped(char **data, size_t *data_length, char *filename, char *url) {
    unsigned int rc = RETURN_OK, content_length = 0, index = 0, checksummed = FALSE;
    unsigned int i = 0, l = 0;
    size_t tar_data_length = 0;
    char *content = NULL, *tar_data = NULL, *shasum = NULL;
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header response_headers = { 0, { }, { } };

    if (!cache_url || (strcmp(url, cache_url) != 0)) {
        /* options */
        download_options.response = &content;
        download_options.response_length = &content_length;
        download_options.url = url;
        download_options.data_length = 1024 * 512;
        download_options.mime_type = MIMETYPE_ZIP;
        download_options.response_headers = &response_headers;

        switch (get_request(download_options)) {
        case 503:
            error_program_log(_("Could not reach the registry"));

            rc = RETURN_EPERM;
            goto cleanup;
        case 404:
            error_program_log(_("Library does not exist"));

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

        cache_url = url;

        cache_content = (char *)nessemble_malloc(sizeof(char) * (content_length + 1));
        memcpy(cache_content, content, content_length);

        cache_content_length = content_length;
        cache_response_headers = response_headers;
    } else {
        content_length = cache_content_length;

        content = (char *)nessemble_malloc(sizeof(char) * (content_length + 1));
        memcpy(content, cache_content, content_length);

        response_headers = cache_response_headers;
    }

    hash(&shasum, content, content_length);

    for (i = 0, l = response_headers.count; i < l; i++) {
        if (strcmp(response_headers.keys[i], "X-Integrity") == 0) {
            if (strcmp(response_headers.vals[i], shasum) == 0) {
                checksummed = TRUE;
            }
        }
    }

    if (checksummed == FALSE) {
        error_program_log(_("Invalid shasum (%s)"), shasum);

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

    if ((rc = get_ungzip(&tar_data, &tar_data_length, content+index, content_length - index)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = untar(&*data, &*data_length, tar_data, tar_data_length, filename)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(tar_data);
    nessemble_free(content);
    nessemble_free(shasum);

    return rc;
}
