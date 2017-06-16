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
#include <sys/stat.h>
#endif /* IS_WINDOWS */

static char *translated_path;

#ifdef IS_WINDOWS
static char *translated;
static unsigned int translatable;
static char *translated_data;
#endif /* IS_WINDOWS */

void translate_init() {
#ifndef IS_WINDOWS
    if (get_home_path(&translated_path, 2, "." PROGRAM_NAME, "locale") != RETURN_OK) {
        return;
    }

    UNUSED(setlocale(LC_ALL, ""));
    UNUSED(bindtextdomain(PROGRAM_NAME, translated_path));
    UNUSED(textdomain(PROGRAM_NAME));

    nessemble_free(translated_path);
#else /* IS_WINDOWS */
    char translated_lang[16];
    size_t translated_data_length = 0;
    FILE *translated_file = NULL;
    struct stat stbuf;

    translated = (char *)nessemble_malloc(sizeof(char) * 256);
    translatable = TRUE;

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, &translated_lang[0], 16);

    if (get_home_path(&translated_path, 5, "." PROGRAM_NAME, "locale", translated_lang, "LC_MESSAGES", PROGRAM_NAME ".mo") != RETURN_OK) {
        translatable = FALSE;
        return;
    }

    if (file_exists(translated_path) != TRUE) {
        translatable = FALSE;
        return;
    }

    if (!(translated_file = fopen(translated_path, "r"))) {
        translatable = FALSE;
        return;
    }

    if (fstat(fileno(translated_file), &stbuf) != 0) {
        translatable = FALSE;
        return;
    }

    translated_data_length = stbuf.st_size;
    translated_data = (char *)nessemble_malloc(sizeof(char) * translated_data_length);

    if (fread(translated_data, 1, translated_data_length, translated_file) != translated_data_length) {
        translatable = FALSE;
        return;
    }

    fclose(translated_file);
#endif /* IS_WINDOWS */
}

void translate_free() {
#ifdef IS_WINDOWS
    nessemble_free(translated);
    nessemble_free(translated_path);
    nessemble_free(translated_data);
#endif /* IS_WINDOWS */
}

char *translate(char *id) {
#ifdef IS_WINDOWS
    unsigned int i = 0, l = 0;
    unsigned int magic = 0, string_count = 0;
    unsigned int offset_ids = 0, offset_strs = 0;
    unsigned int length = 0, offset = 0;
    unsigned int length_id = 0;
    unsigned int (*getu32)(char *str);
    char *rc = id;

    if (translatable == FALSE) {
        goto cleanup;
    }

    magic = getu32_big(translated_data);

    if (magic == 0xDE120495) {
        getu32 = &getu32_big;
    } else {
        getu32 = &getu32_little;
    }

    string_count = (*getu32)(translated_data+8);
    offset_ids = (*getu32)(translated_data+12);
    offset_strs = (*getu32)(translated_data+16);

    length_id = (unsigned int)strlen(id);

    for (i = 0, l = string_count * 8; i < l; i += 8) {
        length = (*getu32)(translated_data+(offset_ids+i));
        offset = (*getu32)(translated_data+(offset_ids+(i+4)));

        if (length != length_id) {
            continue;
        }

        if (strcmp(id, translated_data+offset) != 0) {
            continue;
        }

        length = (*getu32)(translated_data+(offset_strs+i));
        offset = (*getu32)(translated_data+(offset_strs+(i+4)));

        rc = translated_data+offset;
        goto cleanup;
    }

cleanup:
    return rc;
#else
    return gettext(id);
#endif
}
