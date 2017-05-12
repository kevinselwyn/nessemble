#include "../nessemble.h"

/**
 * .inesmap pseudo instruction
 * @param {int} value - Mapper ID
 */
void pseudo_inesmap(unsigned int value) {
    flags |= FLAG_NES;

    ines.map = value;
}
