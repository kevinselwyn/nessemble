#include <stdio.h>
#include <stdlib.h>

/**
 * Exponent math function
 * @param {float} x - Base
 * @param {int} y - Power
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
