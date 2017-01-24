#include "../nessemble.h"

/**
 * .fill pseudo instruction
 * @param {unsigned int} count - Fill count
 * @param {unsigned int} value - Fill value
 */
void pseudo_fill(unsigned int count, unsigned int value) {
    unsigned int i = 0, l = 0;

    for (i = 0, l = count; i < l; i++) {
        write_byte(value);
    }
}
