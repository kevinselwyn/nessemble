#include "../nessemble.h"

/**
 * .inesmir pseudo instruction
 * @param {int} value - Mirroring value
 */
void pseudo_inesmir(unsigned int value) {
    flags |= FLAG_NES;

    ines.mir = value;
}
