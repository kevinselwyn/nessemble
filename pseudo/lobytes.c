#include "../nessemble.h"

/**
 * .lobytes pseudo instruction
 */
void pseudo_lobytes() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}
