#include "../nessemble.h"

/**
 * .ineschr pseudo instruction
 * @param {int} value - Number of 8KB CHR banks
 */
void pseudo_ineschr(unsigned int value) {
    flags |= FLAG_NES;

    ines.chr = value;
}
