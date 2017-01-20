#include "../nessemble.h"

/**
 * .db pseudo instruction
 */
void pseudo_db() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = (unsigned int)length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}
