#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "nessemble.h"

#ifdef IS_WINDOWS
#include <windows.h>
#endif /* IS_WINDOWS */

void *nessemble_malloc(size_t size) {
    void *mem = NULL;

    mem = malloc(sizeof(char) * size);

    if (!mem) {
        fatal(_("Memory error"));
    }

    memset(mem, 0, size);

    return mem;
}

void *nessemble_realloc(void *ptr, size_t size) {
    void *mem = NULL;

    mem = realloc(ptr, sizeof(char) * size);

    if (!mem) {
        fatal(_("Memory error"));
    }

    return mem;
}

void nessemble_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

void nessemble_fclose(FILE *file) {
    if (file) {
        (void)fclose(file);
    }
}

void nessemble_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

char *nessemble_strdup(char *str) {
    char *dup = NULL;
    size_t length = 0;

    length = strlen(str);
    dup = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    strcpy(dup, str);

    return dup;
}

const char *nessemble_strcasestr(const char *s1, const char *s2) {
#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    return strcasestr(s1, s2);
#else /* !IS_WINDOWS && !IS_JAVASCRIPT */
    return strstr(s1, s2);
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */
}

int nessemble_mkdir(const char *dirname, int mode) {
    int rc = 0;

#ifdef IS_WINDOWS
    rc = mkdir(dirname);
#else /* IS_WINDOWS */
    rc = mkdir(dirname, (unsigned int)mode);
#endif /* IS_WINDOWS */

    return rc;
}

int nessemble_rmdir(const char *path) {
    int rc = -1;
    DIR *d = opendir(path);
    size_t path_len = strlen(path);

    if (d) {
        struct dirent *p;
        rc = 0;

        while (!rc && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = nessemble_rmdir(buf);
                    } else {
                        r2 = unlink(buf);
                    }
                }

                free(buf);
            }

            rc = r2;
        }

        closedir(d);
    }

    if (!rc) {
        rc = rmdir(path);
    }

    return rc;
}

char *nessemble_getpass(const char *prompt) {
    char *password = NULL;

#ifdef IS_WINDOWS
    size_t length = 0;

    password = (char *)nessemble_malloc(sizeof(char) * 256);

    fputs(prompt, stdout);

    /* hide cursor */
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);

    while (get_line(&password, NULL) != NULL) {
        length = strlen(password);

        if (length - 1 == 0) {
            continue;
        }

        password[length - 1] = '\0';
        break;
    }

    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
#else /* IS_WINDOWS */
    password = getpass(prompt);
#endif /* IS_WINDOWS */

    return password;
}

char *nessemble_realpath(const char *path, char *resolved_path) {
#ifdef IS_WINDOWS
    char *lppPart = NULL;

    if (GetFullPathName(path, 1024, resolved_path, &lppPart) == 0) {
        return NULL;
    }

    return resolved_path;
#else /* IS_WINDOWS */
    return realpath(path, resolved_path);
#endif /* IS_WINDOWS */
}

unsigned int is_stdout(char *filename) {
    unsigned int rc = FALSE;
    FILE *file = NULL;

    file = fopen(filename, "w+");

    if (!file) {
        goto cleanup;
    }

    if (isatty(fileno(file)) == 1 || strcmp(filename, SEP "dev" SEP "stdout") == 0) {
        rc = TRUE;
    }

cleanup:
    nessemble_fclose(file);

    return rc;
}

/**
 * Convert hex string to int
 * @param {char *} hex - Hexadecimal string (ex: $12)
 * @return {int} Base-10 integer
 */
int hex2int(char *hex) {
    size_t length = strlen(hex);

    if (hex[length - 1] == 'h') {
        hex[length - 1] = '\0';
    }

    return (int)strtol(hex, NULL, 16);
}

/**
 * Convert binary string to int
 * @param {char *} bin - Binary string (ex: %110)
 * @return {int} Base-10 integer
 */
int bin2int(char *bin) {
    size_t length = strlen(bin);

    if (bin[length - 1] == 'b') {
        bin[length - 1] = '\0';
    }

    return (int)strtol(bin, NULL, 2);
}

/**
 * Convert octal string to int
 * @param {char *} oct - Octal string
 * @return {int} Base-10 integer
 */
int oct2int(char *oct) {
    size_t length = strlen(oct);

    if (oct[length - 1] == 'o') {
        oct[length - 1] = '\0';
    }

    return (int)strtol(oct, NULL, 8);
}

/**
 * Convert decimal string to int
 * @param {char *} dec - Decimal string (ex: 34)
 * @return {int} Base-10 integer
 */
int dec2int(char *dec) {
    size_t length = strlen(dec);

    if (dec[length - 1] == 'd') {
        dec[length - 1] = '\0';
    }

    return (int)strtol(dec, NULL, 10);
}

/**
 * Convert defchr string to int
 * @param {char *} dec - defchr string (ex: 123..321)
 * @return {int} Base-10 integer
 */
int defchr2int(char *defchr) {
    unsigned int i = 0, l = 0;
    size_t length = strlen(defchr);

    for (i = 0, l = (unsigned int)length; i < l; i++) {
        if (defchr[i] == '.') {
            defchr[i] = '0';
        }
    }

    return dec2int(defchr);
}

unsigned int fgetu16_little(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);

    return a | (b << 8);
}

unsigned int fgetu16_big(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);

    return (a << 8) | b;
}

unsigned int fgetu32_little(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);
    unsigned int c = (unsigned int)fgetc(fp);
    unsigned int d = (unsigned int)fgetc(fp);

    return a | (b << 8) | (c << 16) | (d << 24);
}

unsigned int fgetu32_big(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);
    unsigned int c = (unsigned int)fgetc(fp);
    unsigned int d = (unsigned int)fgetc(fp);

    return (a << 24) | (b << 16) | (c << 8) | d;
}

unsigned int getu16_little(char *str) {
    unsigned int a = (unsigned int)str[0];
    unsigned int b = (unsigned int)str[1];

    return a | (b << 8);
}

unsigned int getu16_big(char *str) {
    unsigned int a = (unsigned int)str[0];
    unsigned int b = (unsigned int)str[1];

    return (a << 8) | b;
}

unsigned int getu32_little(char *str) {
    unsigned int a = (unsigned int)str[0] & 0xFF;
    unsigned int b = (unsigned int)str[1] & 0xFF;
    unsigned int c = (unsigned int)str[2] & 0xFF;
    unsigned int d = (unsigned int)str[3] & 0xFF;

    return a | (b << 8) | (c << 16) | (d << 24);
}

unsigned int getu32_big(char *str) {
    unsigned int a = (unsigned int)str[0];
    unsigned int b = (unsigned int)str[1];
    unsigned int c = (unsigned int)str[2];
    unsigned int d = (unsigned int)str[3];

    return (a << 24) | (b << 16) | (c << 8) | d;
}

/**
 * Hash string
 * @param {char *} string - String to hash
 * @return {unsigned int} Hash
 */
unsigned int str2hash(char *string) {
    unsigned int hash = 5381, i = 0, l = 0;
    size_t length = strlen(string);

    for (i = 1, l = (unsigned int)length - 1; i < l; i++) {
        hash = ((hash << 5) + hash) + (unsigned int)string[i];
    }

    return hash;
}

/**
 * Base64 encode
 * @param {char **} encoded - Encoded string
 * @param {char *} str - Input string
 */
unsigned int base64enc(char **encoded, char *str) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, j = 0;
    unsigned int a = 0, b = 0, c = 0, abc = 0;
    unsigned int mod_table[3] = { 0, 2, 1 };
    unsigned int input_length = 0, output_length = 0;
    char *output = NULL;
    const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    input_length = (unsigned int)strlen(str);
    output_length = 4 * ((input_length + 2) / 3);

    output = (char *)nessemble_malloc(sizeof(char) * (output_length + 1));

    for (i = 0, j = 0; i < (unsigned int)input_length;) {
        a = i < input_length ? (unsigned int)str[i++] : 0;
        b = i < input_length ? (unsigned int)str[i++] : 0;
        c = i < input_length ? (unsigned int)str[i++] : 0;

        abc = (a << 0x10) + (b << 0x08) + c;

        output[j++] = alpha[(abc >> 18) & 0x3F];
        output[j++] = alpha[(abc >> 12) & 0x3F];
        output[j++] = alpha[(abc >> 6) & 0x3F];
        output[j++] = alpha[abc & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++) {
        output[output_length - 1 - i] = '=';
    }

    *encoded = output;

	return rc;
}

/**
 * Get fullpath
 */
unsigned int get_fullpath(char **path, char *string) {
    unsigned int rc = RETURN_OK;
    size_t string_length = 0, path_length = 0;
    char *fullpath = NULL;

    if (string[0] == '<') {
        if ((rc = get_libpath(path, string)) != RETURN_OK) {
            error(_("Could not include library `%s`"), string);
        }

        goto cleanup;
    }

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    fullpath = (char *)nessemble_malloc(sizeof(char) * (path_length + 1));

    strcpy(fullpath, cwd_path);
    strcat(fullpath, SEP);
    strncat(fullpath, string + 1, string_length - 2);

    if (fullpath[path_length-1] == '"') {
        fullpath[path_length-1] = '\0';
    }

    *path = fullpath;

cleanup:
    return rc;
}

/**
 * Get libpath
 */
unsigned int get_libpath(char **path, char *string) {
    unsigned int rc = RETURN_OK;
    size_t string_length = 0;
    char old = '\0';
    char *fullpath = NULL;

    string_length = strlen(string);
    old = string[string_length-5];
    string[string_length-5] = '\0';

    if ((rc = get_home_path(&fullpath, 4, "." PROGRAM_NAME, "libs", string+1, "lib.asm")) != RETURN_OK) {
        goto cleanup;
    }

    string[string_length-5] = old;

    if (file_exists(fullpath) == FALSE) {
        rc = RETURN_EPERM;
    }

    *path = fullpath;

cleanup:
    return rc;
}

unsigned int file_exists(char *filename) {
    unsigned int exists = FALSE;
    struct stat buffer;

    if (stat(filename, &buffer) == 0) {
        exists = TRUE;
    }

    return exists;
}

/**
 * Load file
 */
unsigned int load_file(char **data, unsigned int *data_length, char *filename) {
    unsigned int rc = RETURN_OK;
    unsigned int insize = 0;
    char *indata = NULL;
    FILE *infile = NULL;
    struct stat stbuf;

    infile = fopen(filename, "r");

    if (!infile) {
        error_program_log(_("Could not open `%s`"), filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((fstat(fileno(infile), &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
        error_program_log(_("Could not get filesize"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    insize = (unsigned int)stbuf.st_size;

    if (insize == 0) {
        error_program_log(_("`%s` is empty"), filename);

        insize = 0;
        rc = RETURN_EPERM;
        goto cleanup;
    }

    indata = (char *)nessemble_malloc(sizeof(char) * (insize + 1));

    if (fread(indata, 1, (size_t)insize, infile) != (size_t)insize) {
        error_program_log(_("Could not read `%s`"), filename);

        insize = 0;
        rc = RETURN_EPERM;
        goto cleanup;
    }

    *data = indata;
    *data_length = insize;

cleanup:
    nessemble_fclose(infile);

    return rc;
}

unsigned int tmp_save(FILE *file, char *filename) {
    unsigned int rc = RETURN_OK;
    char ch = '\0';
    FILE *tmp = NULL;

    tmp = fopen(filename, "w+");

    if (!tmp) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (fread(&ch, 1, 1, file) > 0) {
        UNUSED(fwrite(&ch, 1, sizeof(ch), tmp));
    }

cleanup:
    nessemble_fclose(tmp);

    return rc;
}

void tmp_delete(char *filename) {
    UNUSED(unlink(filename));
}

char *get_line(char **buffer, char *prompt) {
    if (prompt) {
        printf("%s", prompt);
    }

    return fgets(*buffer, 256, stdin);
}

unsigned int parse_extension(char **extension, char *filename) {
    unsigned int rc = RETURN_OK;
    char *dot = strrchr(filename, '.');

    if (dot == NULL || dot == filename) {
        *extension = nessemble_strdup("");
        return rc;
    }

    *extension = nessemble_strdup(dot+1);
    return rc;
}

/**
 * Test if undocumented flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_undocumented() {
    return (unsigned int)((flags & FLAG_UNDOCUMENTED) != 0 ? TRUE : FALSE);
}

/**
 * Test if nes flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_nes() {
    return (unsigned int)((flags & FLAG_NES) != 0 ? TRUE : FALSE);
}

/**
 * Test if disassemble flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_disassemble() {
    return (unsigned int)((flags & FLAG_DISASSEMBLE) != 0 ? TRUE : FALSE);
}

/**
 * Test if reassemble flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_reassemble() {
    return (unsigned int)((flags & FLAG_REASSEMBLE) != 0 ? TRUE : FALSE);
}

/**
 * Test if sumlate flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_simulate() {
    return (unsigned int)((flags & FLAG_SIMULATE) != 0 ? TRUE : FALSE);
}

/**
 * Test if check flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_check() {
    return (unsigned int)((flags & FLAG_CHECK) != 0 ? TRUE : FALSE);
}

/**
 * Test if coverage flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_coverage() {
    return (unsigned int)((flags & FLAG_COVERAGE) != 0 ? TRUE : FALSE);
}

/**
 * Test if CHR segment
 * @return {unsigned int} True if is segment, false if not
 */
unsigned int is_segment_chr() {
    return (unsigned int)(segment_type == SEGMENT_CHR ? TRUE : FALSE);
}

/**
 * Test if PRG segment
 * @return {unsigned int} True if is segment, false if not
 */
unsigned int is_segment_prg() {
    return (unsigned int)(segment_type == SEGMENT_PRG ? TRUE : FALSE);
}
