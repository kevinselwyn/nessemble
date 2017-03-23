#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strings.h"
#include "nessemble.h"

#ifndef IS_WINDOWS
#include <libintl.h>
#include <locale.h>
#else /* IS_WINDOWS */
#include <windows.h>
#endif /* IS_WINDOWS */

static char *translated;
static char *translated_path;

void translate_init() {
    translated = (char *)nessemble_malloc(sizeof(char) * 256);

#ifndef IS_WINDOWS
    if (get_home_path(&translated_path, 2, "." PROGRAM_NAME, "locale") != RETURN_OK) {
        return;
    }

    setlocale(LC_ALL, "");
    bindtextdomain(PROGRAM_NAME, translated_path);
    textdomain(PROGRAM_NAME);

    nessemble_free(translated_path);
#else /* IS_WINDOWS */
    char lang[16];

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, &lang[0], 16);

    if (get_home_path(&translated_path, 5, "." PROGRAM_NAME, "locale", lang, "LC_MESSAGES", PROGRAM_NAME ".mo") != RETURN_OK) {
        return;
    }

    fprintf(stderr, "%s\n", translated_path);
#endif /* IS_WINDOWS */
}

void translate_free() {
    nessemble_free(translated);
    nessemble_free(translated_path);
}

char *translate(char *id) {
#ifdef IS_WINDOWS
    unsigned int string_count = 0;
    FILE *translate_file = NULL;
    unsigned int (*fgetu32)(FILE *) = NULL;

    if (file_exists(translated_path) != TRUE) {
        return id;
    }

    translate_file = fopen(translated_path, "r");

    fprintf(stderr, "0x%08X\n", fgetu32_big(translate_file));

    fclose(translate_file);

    return id;
#else
    return gettext(id);
#endif
}
