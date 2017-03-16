#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

static unsigned int list_index = 0;
static char *list_strings[MAX_LIST_COUNT];

static void list_sort() {
    unsigned int i = 0, l = 0, sorted = FALSE;
    char *tmp = NULL;

    while (sorted == FALSE) {
        sorted = TRUE;

        for (i = 1, l = list_index; i < l; i++) {
            if (strcmp(list_strings[i], list_strings[i-1]) < 0) {
                tmp = nessemble_strdup(list_strings[i]);

                strcpy(list_strings[i], list_strings[i-1]);
                strcpy(list_strings[i-1], tmp);

                sorted = FALSE;
            }
        }
    }
}

unsigned int output_list(char *filename) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int has_constants = FALSE, has_labels = FALSE;
    size_t length = 0;
    FILE *listfile = NULL;

    if (!filename) {
        goto cleanup;
    }

    listfile = fopen(filename, "w");

    if (!listfile) {
        error_program_log("Could not open `%s`", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = symbol_index; i < l; i++) {
        if (symbols[i].type == SYMBOL_CONSTANT || symbols[i].type == SYMBOL_ENUM) {
            has_constants = TRUE;
        }

        if (symbols[i].type == SYMBOL_LABEL || symbols[i].type == SYMBOL_RS) {
            has_labels = TRUE;
        }
    }

    if (has_constants == TRUE) {
        fprintf(listfile, CONSTANTS "\n");

        list_index = 0;

        for (i = 0, l = symbol_index; i < l; i++) {
            if (symbols[i].type == SYMBOL_CONSTANT || symbols[i].type == SYMBOL_ENUM) {
                if (!list_strings[list_index]) {
                    list_strings[list_index] = (char *)nessemble_malloc(sizeof(char) * MAX_LIST_LENGTH);
                }

                length = strlen(symbols[i].name) + 8;

                UNUSED(snprintf(list_strings[list_index++], length, "%04X = %s", symbols[i].value, symbols[i].name));
            }
        }

        list_sort();

        for (i = 0, l = list_index; i < l; i++) {
            fprintf(listfile, "%s\n", list_strings[i]);
        }
    }

    if (has_labels == TRUE) {
        if (has_constants == TRUE) {
            fprintf(listfile, "\n");
        }

        fprintf(listfile, LABELS "\n");

        list_index = 0;

        for (i = 0, l = symbol_index; i < l; i++) {
            if (symbols[i].type == SYMBOL_LABEL || symbols[i].type == SYMBOL_RS) {
                if (!list_strings[list_index]) {
                    list_strings[list_index] = (char *)nessemble_malloc(sizeof(char) * MAX_LIST_LENGTH);
                }

                length = strlen(symbols[i].name) + 11;

                UNUSED(snprintf(list_strings[list_index++], length, "%02X/%04X = %s", symbols[i].bank, symbols[i].value, symbols[i].name));
            }
        }

        list_sort();

        for (i = 0, l = list_index; i < l; i++) {
            fprintf(listfile, "%s\n", list_strings[i]);
        }
    }

cleanup:
    for (i = 0, l = MAX_LIST_COUNT; i < l; i++) {
        nessemble_free(list_strings[i]);
    }

    nessemble_fclose(listfile);

    return rc;
}

unsigned int input_list(char *filename) {
    int symbol_bank = -1;
    unsigned int rc = RETURN_OK, symbol_type = 0, symbol_val = 0;
    char line[MAX_LIST_LENGTH], symbol_name[MAX_LIST_LENGTH];
    FILE *listfile = NULL;

    if (!filename) {
        goto cleanup;
    }

    listfile = fopen(filename, "r");

    if (!listfile) {
        error_program_log("Could not open `%s`", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    symbol_index = 0;

    while (fgets(line, MAX_LIST_LENGTH, listfile)) {
        if (strcmp(line, CONSTANTS "\n") == 0) {
            symbol_type = SYMBOL_CONSTANT;
            continue;
        }

        if (strcmp(line, LABELS "\n") == 0) {
            symbol_type = SYMBOL_LABEL;
            continue;
        }

        if (symbol_type == SYMBOL_CONSTANT) {
            symbols[symbol_index].type = SYMBOL_CONSTANT;
        } else if (symbol_type == SYMBOL_LABEL) {
            symbols[symbol_index].type = SYMBOL_LABEL;
        }

        if (symbol_type == SYMBOL_LABEL || symbol_type == SYMBOL_CONSTANT) {
            if (symbol_type == SYMBOL_CONSTANT) {
                if (sscanf(line, "%04X = %s\n", &symbol_val, symbol_name) != 2) {
                    continue;
                }
            } else {
                if (sscanf(line, "%02X/%04X = %s\n", &symbol_bank, &symbol_val, symbol_name) != 3) {
                    continue;
                }
            }

            symbols[symbol_index].name = nessemble_strdup(symbol_name);
            symbols[symbol_index].value = symbol_val;

            if (symbol_bank >= 0) {
                symbols[symbol_index].bank = (unsigned int)symbol_bank;
                symbol_bank = -1;
            }

            symbol_index++;
        }
    }

cleanup:
    nessemble_fclose(listfile);

    return rc;
}
