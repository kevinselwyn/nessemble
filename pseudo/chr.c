#include "../nessemble.h"

/**
 * .chr[0-9]+ pseudo instruction
 * @param {int} index - CHR bank index
 */
void pseudo_chr(unsigned int index) {
    segment_type = SEGMENT_CHR;

    chr_index = index;
}
