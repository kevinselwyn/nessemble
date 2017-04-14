#include "nessemble.h"
#include "scripts.h"

unsigned int install_scripts() {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    size_t tar_len = 0, tar_filenames_count = 0;
    char *tar = NULL, **tar_filenames = NULL;

    if ((rc = get_ungzip(&tar, &tar_len, (char *)scripts_tar_gz+10, scripts_tar_gz_len-10)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = untar_list(&tar_filenames, &tar_filenames_count, tar, tar_len)) != RETURN_OK) {
        goto cleanup;
    }

    for (i = 0, l = (unsigned int)tar_filenames_count; i < l; i++) {
        fprintf(stderr, "%s\n", tar_filenames[i]);
    }

cleanup:
    return rc;
}
