#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "nessemble.h"

#define REGISTRY_LINE_COUNT  10
#define REGISTRY_LINE_LENGTH 256

#define REGISTRY_CONFIG "config"

static unsigned int open_config(FILE **file, char **filename) {
    unsigned int rc = RETURN_OK;
    char *config_path = NULL;
    size_t length = 0;
    struct passwd *pw = getpwuid(getuid());
    FILE *config = NULL;
    DIR *dir = NULL;

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(pw->pw_dir) + 18;
    config_path = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(config_path, "%s/%s", pw->pw_dir, "." PROGRAM_NAME);

    dir = opendir(config_path);

    if (!dir) {
        if (mkdir(config_path, 0777) != 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    } else {
        (void)closedir(dir);
    }

    strcat(config_path, "/" REGISTRY_CONFIG);

    if (access(config_path, F_OK) == -1) {
        config = fopen(config_path, "w+");
    } else {
        config = fopen(config_path, "r+");
    }

    if (!config) {
        error_program_log("Could not open `" REGISTRY_CONFIG "`");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    *file = config;
    *filename = config_path;

cleanup:
    return rc;
}

static void close_config(FILE *file, char *filename) {
    if (file) {
        (void)fclose(file);
    }

    nessemble_free(filename);
}

static unsigned int get_config(char **result, char *item) {
    unsigned int rc = RETURN_OK, found = FALSE;
    char line[REGISTRY_LINE_LENGTH], key[REGISTRY_LINE_LENGTH], val[REGISTRY_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    while (fgets(line, REGISTRY_LINE_LENGTH, config) != NULL) {
        if (sscanf(line, "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            found = TRUE;
            *result = nessemble_strdup(val);
        }
    }

    if (found == FALSE) {
        error_program_log("No `%s` set", item);

        rc = RETURN_EPERM;
    }

cleanup:
    close_config(config, config_path);

    return rc;
}

unsigned int get_registry(char **registry) {
    return get_config(&*registry, "registry");
}

unsigned int set_registry(char *registry) {
    unsigned int rc = RETURN_OK, found = FALSE, line_index = 0, i = 0, l = 0;
    char lines[REGISTRY_LINE_COUNT][REGISTRY_LINE_LENGTH], key[REGISTRY_LINE_LENGTH], val[REGISTRY_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    while (fgets(lines[line_index], REGISTRY_LINE_LENGTH, config) != NULL) {
        if (sscanf(lines[line_index++], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, "registry") == 0) {
            found = TRUE;
        }
    }

    if (found != TRUE) {
        (void)fclose(config);
        config = fopen(config_path, "a");

        if (!config) {
            error_program_log("Could not open `" REGISTRY_CONFIG "`");

            rc = RETURN_EPERM;
            goto cleanup;
        }

        fprintf(config, "registry\t%s\n", registry);

        goto cleanup;
    }

    (void)fclose(config);
    config = fopen(config_path, "w+");

    if (!config) {
        error_program_log("Could not open `" REGISTRY_CONFIG "`");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = line_index; i < l; i++) {
        if (sscanf(lines[i], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, "registry") == 0) {
            fprintf(config, "%s\t%s\n", key, registry);
        } else {
            fprintf(config, "%s\t%s\n", key, val);
        }
    }

cleanup:
    close_config(config, config_path);

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
    char *dir = NULL;
    struct passwd *pw = getpwuid(getuid());

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(pw->pw_dir) + 11;
    dir = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(dir, "%s/%s", pw->pw_dir, "." PROGRAM_NAME);

    *lib_dir = dir;

cleanup:
    return rc;
}

static unsigned int get_lib_path(char **lib_path, char *lib) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *path = NULL;
    struct passwd *pw = getpwuid(getuid());

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(pw->pw_dir) + strlen(lib) + 16;
    path = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(path, "%s/%s/%s.asm", pw->pw_dir, "." PROGRAM_NAME, lib);

    *lib_path = path;

cleanup:
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

    if (lib_file) {
        (void)fclose(lib_file);
    }

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

    if ((rc = get_lib_dir(&lib_dir)) != RETURN_OK) {
        goto cleanup;
    }

    dp = opendir(lib_dir);

    if (!dp) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_type == DT_REG) {
            length = strlen(ep->d_name);

            if (strcmp(ep->d_name + (length - 4), ".asm") != 0) {
                continue;
            }

            ep->d_name[length - 4] = '\0';

            printf("%s\n", ep->d_name);
        }
    }

    (void)closedir(dp);

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
