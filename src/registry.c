#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "nessemble.h"

unsigned int get_registry(char **registry) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *result = NULL;

    if ((rc = get_config(&result, "registry")) != RETURN_OK) {
        goto cleanup;
    }

    length = strlen(result);

    if (result[length - 1] == '/') {
        result[length - 1] = '\0';
    }

cleanup:
    *registry = result;

    return rc;
}

unsigned int set_registry(char *registry) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;

    length = strlen(registry);

    if (registry[length - 1] == '/') {
        registry[length - 1] = '\0';
    }

    rc = set_config(registry, "registry");

    return rc;
}

static unsigned int get_lib_dir(char **lib_dir) {
    unsigned int rc = RETURN_OK;

    if ((rc = get_home_path(&*lib_dir, 2, "." PROGRAM_NAME, "packages")) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}

static unsigned int get_lib_path(char **lib_path, char *lib) {
    unsigned int rc = RETURN_OK;

    if ((rc = get_home_path(&*lib_path, 3, "." PROGRAM_NAME, "packages", lib)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}

static unsigned int get_lib_file_path(char **lib_file_path, char *lib, char *filename) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *lib_path = NULL, *output = NULL;

    if ((rc = get_lib_path(&lib_path, lib)) != RETURN_OK) {
        goto cleanup;
    }

    length = strlen(lib_path) + strlen(filename);

    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));
    sprintf(output, "%s%s", lib_path, filename);

cleanup:
    *lib_file_path = output;

    nessemble_free(lib_path);

    return rc;
}

static unsigned int lib_is_installed(char *lib) {
    unsigned int installed = FALSE;
    char *lib_path = NULL;

    if (get_lib_file_path(&lib_path, lib, SEP "lib.asm") != RETURN_OK) {
        goto cleanup;
    }

    if (file_exists(lib_path) == FALSE) {
        goto cleanup;
    }

    installed = TRUE;

cleanup:
    nessemble_free(lib_path);

    return installed;
}

unsigned int lib_install(char *lib) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, l = 0;
    size_t lib_length = 0, lib_tar_length = 0;
    size_t lib_path_file_length = 0, max_lib_path_file_length = 0;
    char *lib_files[3] = { "lib.asm", "package.json", "README.md" };
    char *lib_url = NULL, *lib_path = NULL, *lib_path_file = NULL, *lib_zip_url = NULL;
    char *lib_data = NULL, *lib_tar_data = NULL;
    FILE *lib_file = NULL;

    if ((rc = api_lib(&lib_url, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_json_url(&lib_zip_url, "resource", lib_url)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_lib_path(&lib_path, lib)) != RETURN_OK) {
        goto cleanup;
    }

    (void)nessemble_mkdir(lib_path, 0777);

    for (i = 0, l = 3; i < l; i++) {
        lib_path_file_length = strlen(lib_path) + strlen(lib_files[i]);

        if (lib_path_file_length > max_lib_path_file_length) {
            max_lib_path_file_length = lib_path_file_length;
        }
    }

    lib_path_file = (char *)nessemble_malloc(sizeof(char) * (max_lib_path_file_length + 2));

    if ((rc = get_unzipped(&lib_tar_data, &lib_tar_length, lib_zip_url)) != RETURN_OK) {
        goto cleanup;
    }

    for (i = 0, l = 3; i < l; i++) {
        if ((rc = untar(&lib_data, &lib_length, lib_tar_data, lib_tar_length, lib_files[i])) != RETURN_OK) {
            goto cleanup;
        }

        memset(lib_path_file, '\0', max_lib_path_file_length + 2);

        sprintf(lib_path_file, "%s" SEP "%s", lib_path, lib_files[i]);

        lib_file = fopen(lib_path_file, "w+");

        if (!lib_file) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        if (fwrite(lib_data, 1, lib_length, lib_file) != lib_length) {
            rc = RETURN_EPERM;
            goto cleanup;
        };

        nessemble_free(lib_data);
        nessemble_fclose(lib_file);
    }

cleanup:
    nessemble_free(lib_url);
    nessemble_free(lib_path);
    nessemble_free(lib_zip_url);
    nessemble_free(lib_path_file);
    nessemble_free(lib_tar_data);

    return rc;
}

unsigned int lib_uninstall(char *lib) {
    unsigned int rc = RETURN_OK;
    char *lib_path = NULL;

    if ((rc = get_lib_path(&lib_path, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if (lib_is_installed(lib) == FALSE) {
        error_program_log(_("`%s` is not installed"), lib);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (nessemble_rmdir(lib_path) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    nessemble_free(lib_path);

    return rc;
}

unsigned int lib_publish(char *filename, char **package) {
    unsigned int rc = RETURN_OK;
    unsigned int http_code = 0, response_length = 0, data_length = 0;
    char *url = NULL, *response = NULL, *data = NULL, *error = NULL;
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header http_headers = { 0, { }, { } };
    struct http_header response_headers = { 0, { }, { } };

    if ((rc = user_auth(&http_headers, "POST", "/package/publish")) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = api_lib(&url, "publish")) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = load_file(&data, &data_length, filename)) != RETURN_OK) {
        goto cleanup;
    }

    /* options */
    download_options.response = &response;
    download_options.response_length = &response_length;
    download_options.url = url;
    download_options.data = data;
    download_options.data_length = data_length;
    download_options.mime_type = MIMETYPE_ZIP;
    download_options.http_headers = http_headers;
    download_options.response_headers = &response_headers;

    http_code = post_request(download_options);

    if (http_code != 200) {
        if ((rc = get_json_buffer(&error, "error", response)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&*package, "title", response)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int lib_info(char *lib) {
    unsigned int rc = RETURN_OK, readme_length = 0;
    char *lib_url = NULL, *readme = NULL, *readme_url = NULL;
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header response_headers = { 0, { }, { } };

    if (lib_is_installed(lib) == FALSE) {
        if ((rc = api_lib(&lib_url, lib)) != RETURN_OK) {
            goto cleanup;
        }

        if ((rc = get_json_url(&readme_url, "readme", lib_url)) != RETURN_OK) {
            goto cleanup;
        }

        /* options */
        download_options.response = &readme;
        download_options.response_length = &readme_length;
        download_options.url = readme_url;
        download_options.data_length = 1024 * 512;
        download_options.mime_type = MIMETYPE_TEXT;
        download_options.response_headers = &response_headers;

        if (get_request(download_options) != 200) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        if (readme_length == 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        if ((rc = pager_buffer(readme)) != RETURN_OK) {
            goto cleanup;
        }
    } else {
        if ((rc = get_lib_file_path(&lib_url, lib, SEP "README.md")) != RETURN_OK) {
            goto cleanup;
        }

        if ((rc = pager_file(lib_url)) != RETURN_OK) {
            goto cleanup;
        }
    }

cleanup:
    nessemble_free(lib_url);
    nessemble_free(readme_url);
    nessemble_free(readme);

    return rc;
}

unsigned int lib_list() {
    unsigned int rc = RETURN_OK;
    char path[1024];
    char *lib_dir = NULL, *lib_path = NULL, *lib_desc = NULL;
    struct dirent *ep;
    DIR *dp = NULL;
    struct stat s;

    if ((rc = get_lib_dir(&lib_dir)) != RETURN_OK) {
        goto cleanup;
    }

    if ((dp = opendir(lib_dir)) == NULL) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((ep = readdir(dp)) != NULL) {
        memset(path, '\0', 1024);
        sprintf(path, "%s/%s", lib_dir, ep->d_name);

        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0) {
            continue;
        }

        if (file_exists(path) == FALSE) {
            continue;
        }

        UNUSED(stat(path, &s));

        if (!S_ISDIR(s.st_mode)) {
            continue;
        }

        printf("%s", ep->d_name);

        if ((rc = get_lib_file_path(&lib_path, ep->d_name, SEP "package.json")) != RETURN_OK) {
            printf("\n");
            continue;
        }

        if ((rc = get_json_file(&lib_desc, "description", lib_path)) != RETURN_OK) {
            printf("\n");
            continue;
        }

        printf(" - %s\n", lib_desc);

        nessemble_free(lib_path);
        nessemble_free(lib_desc);
    }

    UNUSED(closedir(dp));

cleanup:
    nessemble_free(lib_dir);

    return rc;
}

unsigned int lib_search(char *term) {
    unsigned int rc = RETURN_OK;
    char *lib_search_url = NULL;

    if ((rc = api_search(&lib_search_url, term)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_json_search(lib_search_url, term)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(lib_search_url);

    return rc;
}
