#include "../nessemble.h"

/**
 * .defchr pseudo instruction
 */
void pseudo_defchr() {
    int i = 0, j = 0, k = 0, l = 0, digit = 0;
    unsigned int byte = 0, bit = 0;

    if (length_ints != 8) {
        yyerror("Too few arguments. %d provided, need 8", length_ints);
    }

    while (bit < 2) {
        for (i = 0, j = length_ints; i < j; i++) {
            byte = 0;

            for (k = 7, l = 0; k >= l; k--) {
                digit = (int)(ints[i] / power(10, k)) % 10;
                byte |= ((digit >> bit) & 1) << k;
            }

            write_byte(byte);
        }

        bit++;
    }

    length_ints = 0;
}
