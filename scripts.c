#include "nessemble.h"
#include "scripts.h"

unsigned int install_scripts() {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    size_t tar_len = 0, tar_filenames_count = 0, tar_data_len = 0;
    char *tar = NULL, *tar_data = NULL, *path = NULL;
    char **tar_filenames = NULL;
    FILE *tar_file = NULL;

    if ((rc = get_ungzip(&tar, &tar_len, (char *)scripts_tar_gz+10, scripts_tar_gz_len-10)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = untar_list(&tar_filenames, &tar_filenames_count, tar, tar_len)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = create_config()) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_home_path(&path, 2, "." PROGRAM_NAME, "scripts")) != RETURN_OK) {
        goto cleanup;
    }

    if (nessemble_mkdir(path, 0777) != 0) {
        nessemble_free(path);

        goto cleanup;
    }

    nessemble_free(path);

    for (i = 0, l = (unsigned int)tar_filenames_count; i < l; i++) {
        if ((rc = get_home_path(&path, 2, "." PROGRAM_NAME, tar_filenames[i])) != RETURN_OK) {
            goto cleanup;
        }

        if ((rc = untar(&tar_data, &tar_data_len, tar, tar_len, tar_filenames[i])) != RETURN_OK) {
            goto cleanup;
        }

        tar_file = fopen(path, "w");

        if (fwrite(tar_data, 1, tar_data_len, tar_file) != tar_data_len) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        nessemble_free(path);
        nessemble_free(tar_data);
        nessemble_fclose(tar_file);
    }

cleanup:
    nessemble_free(tar);

    for (i = 0, l = (unsigned int)tar_filenames_count; i < l; i++) {
        nessemble_free(tar_filenames[i]);
    }

    nessemble_free(tar_filenames);

    return rc;
}