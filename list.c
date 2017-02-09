#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

#define MAX_LIST_COUNT  65536
#define MAX_LIST_LENGTH 256

static unsigned int list_index = 0;
static char *list_strings[MAX_LIST_COUNT];

static void list_sort() {
    unsigned int i = 0, l = 0, sorted = FALSE;
    char *tmp = NULL;

    while (sorted == FALSE) {
        sorted = TRUE;

        for (i = 1, l = list_index; i < l; i++) {
            if (strcmp(list_strings[i], list_strings[i-1]) < 0) {
                tmp = strdup(list_strings[i]);

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
        fprintf(stderr, "Could not open `%s`\n", filename);

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
        fprintf(listfile, "[constants]\n");

        list_index = 0;

        for (i = 0, l = symbol_index; i < l; i++) {
            if (symbols[i].type == SYMBOL_CONSTANT || symbols[i].type == SYMBOL_ENUM) {
                if (!list_strings[list_index]) {
                    list_strings[list_index] = (char *)malloc(sizeof(char) * MAX_LIST_LENGTH);
                }

                length = strlen(symbols[i].name) + 7;

                (void)snprintf(list_strings[list_index++], length, "%04X = %s", symbols[i].value, symbols[i].name);
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

        fprintf(listfile, "[labels]\n");

        list_index = 0;

        for (i = 0, l = symbol_index; i < l; i++) {
            if (symbols[i].type == SYMBOL_LABEL || symbols[i].type == SYMBOL_RS) {
                if (!list_strings[list_index]) {
                    list_strings[list_index] = (char *)malloc(sizeof(char) * MAX_LIST_LENGTH);
                }

                length = strlen(symbols[i].name) + 7;

                (void)snprintf(list_strings[list_index++], length, "%04X = %s", symbols[i].value, symbols[i].name);
            }
        }

        list_sort();

        for (i = 0, l = list_index; i < l; i++) {
            fprintf(listfile, "%s\n", list_strings[i]);
        }
    }

cleanup:
    for (i = 0, l = MAX_LIST_COUNT; i < l; i++) {
        if (list_strings[i] != NULL) {
            free(list_strings[i]);
        }
    }

    if (listfile) {
        (void)fclose(listfile);
    }

    return rc;
}
