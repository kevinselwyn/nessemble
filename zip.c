#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include "nessemble.h"

#define ZIP_BUF_SIZE 512 * 1024
#define MIMETYPE_ZIP "application/tar+gzip"

unsigned int get_unzipped(char **data, size_t *data_length, char *filename, char *url) {
    unsigned int rc = RETURN_OK;
    size_t content_length = 0, entry_length = 0;
    char buffer[ZIP_BUF_SIZE];
    char *content = NULL, *unzipped = NULL;
    struct archive *arch = archive_read_new();
    struct archive_entry *entry;

    if (get_request(&content, &content_length, url, MIMETYPE_ZIP) != RETURN_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    archive_read_support_filter_gzip(arch);
    archive_read_support_format_tar(arch);

    if (archive_read_open_memory(arch, content, content_length) != RETURN_OK) {
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

    strcpy(unzipped, buffer);

    *data = unzipped;
    *data_length = entry_length;

cleanup:
    if (arch) {
        archive_read_free(arch);
    }

    return rc;
}
