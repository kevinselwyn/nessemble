#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strings.h"
#include "nessemble.h"

static char *translated;

void translate_free() {
    if (translated) {
        free(translated);
    }
}

char *translate(char *id) {
    unsigned int next = FALSE;
    char *current_line = NULL, *next_line = NULL, *output = NULL;
    char whitespace[32], key[128], val[128];

    translate_free();

    translated = (char *)malloc(sizeof(char) * 128);
    memset(translated, '\0', 128);

    current_line = strings_json;

    while (current_line) {
        next_line = strchr(current_line, '\n');

        if (next_line) {
            *next_line = '\0';
        }

        sscanf(current_line, "%[^\"]\"%[^\"]\": \"%[^\"]\"", whitespace, key, val);

        if (next == TRUE && key[0] != '\0' && strcmp(key, "str") == 0) {
            memcpy(translated, val, 128);
            goto cleanup;
        }

        if (key[0] != '\0' && strcmp(key, "id") == 0 && strcmp(val, id) == 0) {
            next = TRUE;
        }

        if (next_line) {
            *next_line = '\n';
        }

        current_line = next_line ? (next_line + 1) : NULL;

        memset(whitespace, '\0', 32);
        memset(key, '\0', 128);
        memset(val, '\0', 128);
    }

cleanup:
    return translated;
}
