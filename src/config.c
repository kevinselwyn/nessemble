#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "nessemble.h"

static struct config_type {
    char *type;
};

static struct config_type config_types[CONFIG_TYPES] = {
    { "registry" }
};

unsigned int create_config() {
    unsigned int rc = RETURN_OK;
    char *config_path = NULL, *lib_path = NULL;
    DIR *config_dir = NULL, *lib_dir = NULL;

    if ((rc = get_home_path(&config_path, 1, "." PROGRAM_NAME)) != RETURN_OK) {
        goto cleanup;
    }

    if (!config_path) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    config_dir = opendir(config_path);

    if (!config_dir) {
        if (nessemble_mkdir(config_path, 0777) != 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        config_dir = opendir(config_path);
    }

    if ((rc = get_home_path(&lib_path, 2, "." PROGRAM_NAME, "packages")) != RETURN_OK) {
        goto cleanup;
    }

    if (!lib_path) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    lib_dir = opendir(lib_path);

    if (!lib_dir) {
        if (nessemble_mkdir(lib_path, 0777) != 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        lib_dir = opendir(lib_path);
    }

cleanup:
    nessemble_free(config_path);
    nessemble_free(lib_path);

    if (config_dir) {
        UNUSED(closedir(config_dir));
    }

    if (lib_dir) {
        UNUSED(closedir(lib_dir));
    }

    return rc;
}

unsigned int open_config(FILE **file, char **filename) {
    unsigned int rc = RETURN_OK;
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = create_config()) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = get_home_path(&config_path, 2, "." PROGRAM_NAME, CONFIG_FILENAME)) != RETURN_OK) {
        goto cleanup;
    }

    if (!config_path) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (file_exists(config_path) == FALSE) {
        if ((config = fopen(config_path, "w+")) == NULL) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        fprintf(config, "registry\t%s\n", CONFIG_API_DEFAULT);
        (void)fseek(config, 0, SEEK_SET);
    } else {
        config = fopen(config_path, "r+");
    }

    if (!config) {
        error_program_log(_("Could not open `%s`"), CONFIG_FILENAME);

        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    *file = config;
    *filename = config_path;

    return rc;
}

void close_config(FILE *file, char *filename) {
    nessemble_free(filename);
    nessemble_fclose(file);
}

unsigned int get_config(char **result, char *item) {
    unsigned int rc = RETURN_OK, found = FALSE;
    char line[CONFIG_LINE_LENGTH], key[CONFIG_LINE_LENGTH], val[CONFIG_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    if (!config) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!config_path) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (fgets(line, CONFIG_LINE_LENGTH, config) != NULL) {
        if (sscanf(line, "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            found = TRUE;
            *result = nessemble_strdup(val);
        }
    }

    if (found == FALSE) {
        error_program_log(_("No `%s` set"), item);

        rc = RETURN_EPERM;
    }

cleanup:
    close_config(config, config_path);

    return rc;
}

unsigned int set_config(char *result, char *item) {
    unsigned int rc = RETURN_OK, found = FALSE, line_index = 0, i = 0, l = 0;
    char lines[CONFIG_LINE_COUNT][CONFIG_LINE_LENGTH], key[CONFIG_LINE_LENGTH], val[CONFIG_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    if (!config) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (fgets(lines[line_index], CONFIG_LINE_LENGTH, config) != NULL) {
        if (sscanf(lines[line_index++], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            found = TRUE;
        }
    }

    if (found != TRUE) {
        nessemble_fclose(config);

        if (!config_path) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        config = fopen(config_path, "a");

        if (!config) {
            error_program_log(_("Could not open `%s`"), CONFIG_FILENAME);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        if (result != NULL) {
            fprintf(config, "%s\t%s\n", item, result);
        }

        goto cleanup;
    }

    nessemble_fclose(config);

    if (!config_path) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    config = fopen(config_path, "w+");

    if (!config) {
        error_program_log(_("Could not open `%s`"), CONFIG_FILENAME);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = line_index; i < l; i++) {
        if (sscanf(lines[i], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            if (result != NULL) {
                fprintf(config, "%s\t%s\n", key, result);
            }
        } else {
            fprintf(config, "%s\t%s\n", key, val);
        }
    }

cleanup:
    close_config(config, config_path);

    return rc;
}

unsigned int list_config(char **result) {
    unsigned int rc = RETURN_OK, longest = 0, length = 0, i = 0, l = 0;
    char *output = NULL, *val = NULL;

    for (i = 0, l = CONFIG_TYPES; i < l; i++) {
        length = (unsigned int)strlen(config_types[i].type);

        if (length > longest) {
            longest = length + 1;
        }
    }

    length = (longest + CONFIG_LINE_LENGTH + 2) * CONFIG_TYPES;
    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    for (i = 0, l = CONFIG_TYPES; i < l; i++) {
        if ((rc = get_config(&val, config_types[i].type)) != RETURN_OK) {
            goto cleanup;
        }

        if (!val) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        sprintf(output+strlen(output), "%s%*s%s\n", config_types[i].type, (int)(longest - strlen(config_types[i].type)), " ", val);
        nessemble_free(val);
    }

    output[strlen(output)-1] = '\0';

cleanup:
    *result = output;

    return rc;
}
