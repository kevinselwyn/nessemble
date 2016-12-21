#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

/**
 * Handle EOF
 */
void assemble_feof() {
    if (--include_stack_ptr >= 0) {
        include_file_pop();
    }
}

/**
 * End parser pass
 */
void end_pass() {
    pass++;
}

/**
 * Get ROM index
 * @return {int} ROM index
 */
int get_rom_index() {
    int index = 0;

    if (is_segment_prg()) {
        index = prg_offsets[prg_index] + (prg_index * BANK_PRG);
    }

    if (is_segment_chr()) {
        index = chr_offsets[chr_index] + (ines.prg * BANK_PRG);
    }

    return index;
}
