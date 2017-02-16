#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

#define MAX_MACRO_NAME 1024
#define MAX_MACRO_TEXT 1024 * 1024

void add_macro(char *name) {
    macros[macro_index].name = (char *)malloc(sizeof(char) * MAX_MACRO_NAME);

    if (!macros[macro_index].name) {
        fatal("Memory error");
    }

    strcpy(macros[macro_index].name, name);

    macros[macro_index].text = (char *)malloc(sizeof(char) * MAX_MACRO_TEXT);

    if (!macros[macro_index].text) {
        fatal("Memory error");
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
