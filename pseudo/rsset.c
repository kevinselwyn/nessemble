#include "../nessemble.h"

/**
 * .rsset pseudo instruction
 * @param {int} address - Variable start address
 */
void pseudo_rsset(unsigned int address) {
    rsset = address;
}
