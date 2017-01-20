#include "../nessemble.h"

/**
 * .dw pseudo instruction
 */
void pseudo_dw() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
        write_byte((ints[i] >> 8) & 0xFF);
    }

    length_ints = 0;
}
