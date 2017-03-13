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

    rc = get_config(&result, "registry");

    length = strlen(result);

    if (result[length - 1] == '/') {
        result[length - 1] = '\0';
    }

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

static unsigned int get_lib_url(char **lib_url, char *lib) {
    unsigned int rc = RETURN_OK;
    size_t length_registry = 0, length_lib = 0;
    char *registry = NULL, *path = NULL;

    if ((rc = get_registry(&registry)) != RETURN_OK) {
        goto cleanup;
    }

    length_registry = strlen(registry);
    length_lib = strlen(lib);
    path = (char *)nessemble_malloc(sizeof(char) * (length_registry + length_lib + 6) + 1);

    sprintf(path, "%s/%s.json", registry, lib);

    *lib_url = path;

cleanup:
    nessemble_free(registry);

    return rc;
}

static unsigned int get_lib_search_url(char **lib_search_url, char *term) {
    unsigned int rc = RETURN_OK;
    size_t length_registry = 0, length_lib = 0;
    char *registry = NULL, *path = NULL;

    if ((rc = get_registry(&registry)) != RETURN_OK) {
        goto cleanup;
    }

    length_registry = strlen(registry);
    length_lib = strlen(term);
    path = (char *)nessemble_malloc(sizeof(char) * (length_registry + length_lib + 8) + 1);

    if (strcmp(term, ".") == 0) {
        sprintf(path, "%s/", registry);
    } else {
        sprintf(path, "%s/search/%s", registry, term);
    }

    *lib_search_url = path;

cleanup:
    nessemble_free(registry);

    return rc;
}

static unsigned int get_lib_dir(char **lib_dir) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *home = NULL, *dir = NULL;

    if ((rc = get_home(&home)) != RETURN_OK) {
        goto cleanup;
    }

    length = strlen(home) + 11;
    dir = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(dir, "%s/%s", home, "." PROGRAM_NAME);

    *lib_dir = dir;

cleanup:
    nessemble_free(home);

    return rc;
}

static unsigned int get_lib_path(char **lib_path, char *lib) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *home = NULL, *path = NULL;

    if ((rc = get_home(&home)) != RETURN_OK) {
        goto cleanup;
    }

    length = strlen(home) + strlen(lib) + 16;
    path = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(path, "%s/%s/%s.asm", home, "." PROGRAM_NAME, lib);

    *lib_path = path;

cleanup:
    nessemble_free(home);

    return rc;
}

static unsigned int lib_is_installed(char *lib) {
    unsigned int installed = FALSE;
    char *lib_path = NULL;

    if (get_lib_path(&lib_path, lib) != RETURN_OK) {
        goto cleanup;
    }

    if (access(lib_path, F_OK) == -1) {
        goto cleanup;
    }

    installed = TRUE;

cleanup:
    nessemble_free(lib_path);

    return installed;
}

unsigned int lib_install(char *lib) {
    unsigned int rc = RETURN_OK;
    size_t lib_length = 0;
    char *lib_url = NULL, *lib_path = NULL, *lib_zip_url = NULL, *lib_data = NULL;
    FILE *lib_file = NULL;

    if ((rc = get_lib_url(&lib_url, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_json(&lib_zip_url, "resource", lib_url)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_lib_path(&lib_path, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_unzipped(&lib_data, &lib_length, "lib.asm", lib_zip_url)) != RETURN_OK) {
        goto cleanup;
    }

    lib_file = fopen(lib_path, "w+");

    if (!lib_file) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (fwrite(lib_data, 1, lib_length, lib_file) != lib_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    };

cleanup:
    nessemble_free(lib_url);
    nessemble_free(lib_path);
    nessemble_free(lib_zip_url);
    nessemble_free(lib_data);
    nessemble_fclose(lib_file);

    return rc;
}

unsigned int lib_uninstall(char *lib) {
    unsigned int rc = RETURN_OK;
    char *lib_path = NULL;

    if ((rc = get_lib_path(&lib_path, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if (lib_is_installed(lib) == FALSE) {
        error_program_log("`%s` is not installed", lib);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (unlink(lib_path) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    nessemble_free(lib_path);

    return rc;
}

unsigned int lib_info(char *lib) {
    unsigned int rc = RETURN_OK, readme_length = 0, http_code = 0;
    char *lib_url = NULL, *readme = NULL, *readme_url = NULL;

    if ((rc = get_lib_url(&lib_url, lib)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_json(&readme_url, "readme", lib_url)) != RETURN_OK) {
        goto cleanup;
    }

    http_code = get_request(&readme, &readme_length, readme_url, MIMETYPE_TEXT);

    if (http_code != 200) {
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

cleanup:
    nessemble_free(lib_url);
    nessemble_free(readme_url);
    nessemble_free(readme);

    return rc;
}

unsigned int lib_list() {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *lib_dir = NULL;
    struct dirent *ep;
    DIR *dp = NULL;
    struct stat s;

    if ((rc = get_lib_dir(&lib_dir)) != RETURN_OK) {
        goto cleanup;
    }

    dp = opendir(lib_dir);

    if (!dp) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((ep = readdir(dp)) != NULL) {
        stat(ep->d_name, &s);

        if ((s.st_mode & S_IFDIR) != 0) {
            length = strlen(ep->d_name);

            if (strcmp(ep->d_name + (length - 4), ".asm") != 0) {
                continue;
            }

            ep->d_name[length - 4] = '\0';

            printf("%s\n", ep->d_name);
        }
    }

    UNUSED(closedir(dp));

cleanup:
    nessemble_free(lib_dir);

    return rc;
}

unsigned int lib_search(char *term) {
    unsigned int rc = RETURN_OK;
    char *lib_search_url = NULL;

    if ((rc = get_lib_search_url(&lib_search_url, term)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_json_search(lib_search_url, term)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(lib_search_url);

    return rc;
}
