#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strings.h"
#include "nessemble.h"

#ifndef IS_WINDOWS
#include <libintl.h>
#include <locale.h>
#endif /* IS_WINDOWS */

static char *translated;

void translate_init() {
    translated = (char *)nessemble_malloc(sizeof(char) * 256);

#ifndef IS_WINDOWS
    char *path = NULL;

    if (get_home_path(&path, 2, "." PROGRAM_NAME, "locale") != RETURN_OK) {
        return;
    }

    fprintf(stderr, "%s\n", path);

    setlocale(LC_ALL, "");
    bindtextdomain(PROGRAM_NAME, path);
    textdomain(PROGRAM_NAME);

    nessemble_free(path);
#endif /* IS_WINDOWS */
}

void translate_free() {
    nessemble_free(translated);
}

char *translate(char *id) {
#ifdef IS_WINDOWS
    return id;
#else
    return gettext(id);
#endif
}
