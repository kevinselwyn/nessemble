#include <string.h>
#include "../nessemble.h"

/**
 * .out pseudo instruction
 * @param {char *} string - Text to output
 */
void pseudo_out(char *string) {
    size_t length = 0;

    if (pass == 2) {
        length = strlen(string);
        string[length - 1] = '\0';

        fprintf(stderr, "%s\n", string+1);
    }
}
