#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int custom(char **return_str, size_t *return_len, unsigned int ints[], int length_ints, char *texts[], int length_texts) {
    int arg = 0;
    unsigned int quotient = 0, i = 0, l = 0;
    char *str = NULL;

    for (i = 0, l = length_ints; i < l; i++) {
        arg = ints[i];

        if (i == 0) {
            quotient = arg;
        } else {
            quotient = (unsigned int)((double)quotient / (double)arg);
        }
    }

    str = (char *)malloc(sizeof(char) * 2);
    str[0] = quotient % 256;

    *return_str = str;
    *return_len = 1;

    return 0;
}
