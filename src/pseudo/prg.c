#include "../nessemble.h"

/**
 * .prg[0-9]+ pseudo instruction
 * @param {int} index - PRG bank index
 */
void pseudo_prg(unsigned int index) {
    segment_type = SEGMENT_PRG;

    prg_index = index;
}
