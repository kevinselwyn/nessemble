#include "../nessemble.h"

/**
 * .org pseudo instruction
 * @param {int} address - Organization address
 */
void pseudo_org(unsigned int address) {
    // TODO: add check for too high an address

    if (is_segment_prg() == TRUE) {
        if (ines.prg < 2) {
            prg_offsets[prg_index] = address - 0xC000;
        } else {
            prg_offsets[prg_index] = address - 0x8000;
        }
    }

    if (is_segment_chr() == TRUE) {
        chr_offsets[chr_index] = address;
    }
}
