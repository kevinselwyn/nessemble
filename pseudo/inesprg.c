#include "../nessemble.h"

/**
 * .inesprg pseudo instruction
 * @param {int} value - Number of 16KB PRG banks
 */
void pseudo_inesprg(unsigned int value) {
    flags |= FLAG_NES;

    ines.prg = value;
}
