#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include "nessemble.h"

#define ZIP_BUF_SIZE 512 * 1024
#define MIMETYPE_ZIP "application/tar+gzip"

unsigned int get_unzipped(char **data, size_t *data_length, char *filename, char *url) {
    unsigned int rc = RETURN_OK, http_code = 0, content_length = 0;
    size_t entry_length = 0;
    char buffer[ZIP_BUF_SIZE];
    char *content = NULL, *unzipped = NULL;
    struct archive *arch = archive_read_new();
    struct archive_entry *entry;

    http_code = get_request(&content, &content_length, url, MIMETYPE_ZIP);

    fprintf(stderr, "%d\n", content_length);

    switch (http_code) {
    case 503:
        fprintf(stderr, "Could not reach the registry\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 404:
        fprintf(stderr, "Library does not exist\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 200:
    default:
        break;
    }

    archive_read_support_filter_gzip(arch);
    archive_read_support_format_tar(arch);

    if (archive_read_open_memory(arch, content, content_length) != RETURN_OK) {
        fprintf(stderr, "ERR\n");
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (archive_read_next_header(arch, &entry) == ARCHIVE_OK) {
        if (strcmp(archive_entry_pathname(entry), filename) != 0) {
            archive_read_data_skip(arch);
            continue;
        }

        entry_length = archive_read_data(arch, buffer, ZIP_BUF_SIZE);
        break;
    }

    if (entry_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    unzipped = (char *)malloc(sizeof(char) * (entry_length + 1));

    if (!unzipped) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    memcpy(unzipped, buffer, entry_length + 1);

cleanup:
    *data = unzipped;
    *data_length = entry_length;

    if (arch) {
        archive_read_free(arch);
    }

    if (content) {
        free(content);
    }

    return rc;
}
