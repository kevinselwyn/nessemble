#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

void add_macro(char *name) {
    macros[macro_index].name = (char *)nessemble_malloc(sizeof(char) * MAX_MACRO_NAME);
    strcpy(macros[macro_index].name, name);

    macros[macro_index].text = (char *)nessemble_malloc(sizeof(char) * MAX_MACRO_TEXT);
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

char *argu_macro(char *name) {
    size_t length = 0;
    char old_name[128], num[32];

    memset(num, '\0', 32);
    sprintf(num, "%d", arg_unique);

    strcpy(old_name, name);

    length = strlen(name) + strlen(num);
    name = (char *)realloc(name, sizeof(char) * (length + 1));
    memset(name, '\0', length);

    sprintf(name, "%s%d", old_name, arg_unique);

    return name;
}
