#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

void add_macro(char *name) {
    macros[macro_index].name = name;
    macros[macro_index].text = (char *)malloc(sizeof(char) * (1024 * 1024));
}

void end_macro() {
    fprintf(stderr, "Macro: %s\n%s", macros[macro_index].name, macros[macro_index].text);
    macro_index++;
}

void append_macro(char *text) {
    strcat(macros[macro_index].text, text);
    strcat(macros[macro_index].text, "\n");
}
