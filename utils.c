#include <stdio.h>
#include <stdlib.h>
#include "nessemble.h"

/**
 * Exponent math function
 * @param {float} x - Base
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
 * Parse int from string
 * @param {char *} text - Text to parse
 * @return {int} Integer
 */
int parse2int(char *text) {
    return (int)strtol(text+4, NULL, 10);
}

/**
 * Convert hex string to int
 * @param {char *} hex - Hexadecimal string (ex: $12)
 * @return {int} Base-10 integer
 */
int hex2int(char *hex) {
    return (int)strtol(hex+1, NULL, 16);
}

/**
 * Convert binary string to int
 * @param {char *} bin - Binary string (ex: %110)
 * @return {int} Base-10 integer
 */
int bin2int(char *bin) {
    return (int)strtol(bin+1, NULL, 2);
}

/**
 * Convert octal string to int
 * @param {char *} oct - Octal string
 * @return {int} Base-10 integer
 */
int oct2int(char *oct) {
    return (int)strtol(oct, NULL, 8);
}

/**
 * Convert decimal string to int
 * @param {char *} dec - Decimal string (ex: 34)
 * @return {int} Base-10 integer
 */
int dec2int(char *dec) {
    return atoi(dec);
}

/**
 * Load file
 */
size_t load_file(char **data, char *filename) {
    size_t insize = 0;
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

    insize = (size_t)ftell(infile);

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

    if (fread(indata, 1, insize, infile) != insize) {
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
