#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include "nessemble.h"

/**
 * Exponent math function
 * @param {int} x - Base
 * @param {int} y - Power
 * @return {int} Result
 */
int power(int x, int y) {
    int temp = 0;

    if (y == 0) {
        return 1;
    }

    temp = power(x, (int)(y / 2));

    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0) {
            return x * temp * temp;
        } else {
            return (int)((temp * temp) / x);
        }
    }
}

/**
 * Square root math function
 * @param {int} x - Base
 * @param {int} y - Power
 * @return {int} Result
 */
int root(int x, int y) {
    int d = 0, r = 1;

    if (x == 0) {
        return 0;
    }

    if (y < 1 || (x < 0 && (y & 1) == 0)) {
        return 0;
    }

    do {
        d = (x / power(r, y - 1) - r) / y;
        r += d;
    } while ((double)d >= (DBL_EPSILON * 10) || (double)d <= (-DBL_EPSILON * 10));

    return r;
}

/**
 * Calculate CRC-32
 * @param {unsigned int *} buffer - Data buffer
 * @param {unsigned int} length - Data length
 * @return {unsigned int} CRC-32 value
 */
unsigned int crc_32(unsigned int *buffer, unsigned int length) {
    unsigned int crc = 0;
    unsigned int has_table = FALSE, rem = 0, octet = 0, i = 0, j = 0;
    unsigned int table[256];
    unsigned int *p, *q;

    if (has_table == FALSE) {
        for (i = 0; i < 256; i++) {
            rem = i;

            for (j = 0; j < 8; j++) {
                if ((rem & 1) != 0) {
                    rem >>= 1;
                    rem ^= 0xEDB88320;
                } else {
                    rem >>= 1;
                }
            }

            table[i] = rem;
        }

        has_table = TRUE;
    }

    crc = ~crc;
    q = buffer + length;

    for (p = buffer; p < q; p++) {
        octet = *p;
        crc = (crc >> 8) ^ table[(crc & 0xFF) ^ octet];
    }

    return ~crc;
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
 * Get fullpath
 */
int get_fullpath(char **path, char *string) {
    int rc = RETURN_OK;
    size_t string_length = 0, path_length = 0;
    char *fullpath = NULL;

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    fullpath = (char *)malloc(sizeof(char) * (path_length + 1));

    if (!fullpath) {
        yyerror("Memory error");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    strcpy(fullpath, cwd_path);
    strcat(fullpath, "/");
    strncat(fullpath, string + 1, string_length - 2);

    if (fullpath[path_length-1] == '"') {
        fullpath[path_length-1] = '\0';
    }

    *path = fullpath;

cleanup:
    return rc;
}

/**
 * Load file
 */
unsigned int load_file(char **data, char *filename) {
    unsigned int insize = 0;
    char *indata = NULL;
    FILE *infile = NULL;

    infile = fopen(filename, "r");

    if (!infile) {
        fprintf(stderr, "Could not open %s\n", filename);
        goto cleanup;
    }

    if (fseek(infile, 0, SEEK_END) != 0) {
        fprintf(stderr, "Seek error\n");
        goto cleanup;
    }

    insize = (unsigned int)ftell(infile);

    if (fseek(infile, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Seek error\n");

        insize = 0;
        goto cleanup;
    }

    if (insize == 0) {
        fprintf(stderr, "%s is empty\n", filename);

        insize = 0;
        goto cleanup;
    }

    indata = (char *)malloc(sizeof(char) * (insize + 1));

    if (!indata) {
        fprintf(stderr, "Memory error\n");

        insize = 0;
        goto cleanup;
    }

    if (fread(indata, 1, (size_t)insize, infile) != (size_t)insize) {
        fprintf(stderr, "Could not read %s\n", filename);

        insize = 0;
        goto cleanup;
    }

    *data = indata;

cleanup:
    if (infile) {
        (void)fclose(infile);
    }

    return insize;
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

/**
 * Parser error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void yyerror(const char *fmt, ...) {
    int rc = RETURN_EPERM;

    va_list argptr;
    va_start(argptr, fmt);

    fprintf(stderr, "Error in `%s` on line %d: ", filename_stack[include_stack_ptr], yylineno);
    (void)vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}
