#include "../nessemble.h"

/**
 * .defchr pseudo instruction
 */
void pseudo_defchr() {
    unsigned int i = 0, j = 0, k = 0, l = 0, loops = 2, digit = 0;
    unsigned int byte = 0;

    if (length_ints != 8) {
        yyerror("Too few arguments. %d provided, need 8", length_ints);
    }

    while (loops != 0) {
        for (i = 0, j = 8; i < j; i++) {
            byte = 0;

            for (k = 0, l = 8; k < l; k++) {
                digit = (ints[i] / power(10, (int)(l - k - 1))) % 10;
                digit = (digit >> (loops - 1)) & 0x01;

                byte |= (digit << (l - k - 1));
            }

            write_byte(byte);
        }

        loops -= 1;
    }

    length_ints = 0;
}
