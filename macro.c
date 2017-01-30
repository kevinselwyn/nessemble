#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

void add_macro(char *name) {
    macros[macro_index].name = (char *)malloc(sizeof(char) * 1024);

    if (!macros[macro_index].name) {
        yyerror("Memory error");
    }

    strcpy(macros[macro_index].name, name);

    macros[macro_index].text = (char *)malloc(sizeof(char) * (1024 * 1024));

    if (!macros[macro_index].text) {
        yyerror("Memory error");
    }

    strcpy(macros[macro_index].text, "");
}

void end_macro() {
    macro_index++;
}

void append_macro(char *text) {
    if (!macros[macro_index].text) {
        return;
    }

    strcat(macros[macro_index].text, text);
    strcat(macros[macro_index].text, "\n");
}

int get_macro(char *name) {
    int index = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = macro_index; i < l; i++) {
        if (strcmp(macros[i].name, name) == 0) {
            index = (int)i;
            break;
        }
    }

    return index;
}

void pseudo_macro(char *string) {
    int index = get_macro(string);

    if (index == -1) {
        error_add("Macro `%s` was not defined", string);
        goto cleanup;
    }

    include_string_push(macros[index].text);

cleanup:
    return;
}
