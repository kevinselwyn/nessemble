#include <string.h>
#include "../nessemble.h"

/**
 * .ascii pseudo instruction
 * @param {char *} string - ASCII string
 * @param {int} offset - Offset
 */
void pseudo_ascii(char *string, int offset) {
    unsigned int i = 0, l = 0;
    size_t length = 0;

    length = strlen(string);

    for (i = 1, l = (unsigned int)length - 1; i < l; i++) {
        write_byte((unsigned int)string[i] + offset);
    }
}
