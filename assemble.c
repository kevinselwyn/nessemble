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
unsigned int get_rom_index() {
    unsigned int index = 0;

    if (is_segment_prg() == TRUE) {
        index = prg_offsets[prg_index] + (prg_index * BANK_PRG);
    }

    if (is_segment_chr() == TRUE) {
        index = chr_offsets[chr_index] + (ines.prg * BANK_PRG);

        if (chr_index > 0) {
            index += chr_index * BANK_CHR;
        }
    }

    return index;
}

/**
 * Get address offset
 * @return {int} PC offset
 */
unsigned int get_address_offset() {
    unsigned int offset = 0;

    if (is_flag_nes() == TRUE) {
        if (is_segment_prg() == TRUE) {
            if (ines.prg < 2) {
                offset = prg_offsets[prg_index] + 0xC000;
            } else {
                offset = prg_offsets[prg_index] + (0x8000 + ((ines.prg % 2) * 0x4000));
            }
        }
    } else {
        offset = prg_offsets[prg_index];
    }

    if (is_segment_chr() == TRUE) {
        offset = chr_offsets[chr_index] + (ines.prg * BANK_PRG);

        if (chr_index > 0) {
            offset += chr_index * BANK_CHR;
        }
    }

    return offset;
}

/**
 * Write byte
 * @param {unsigned int} byte - Byte to write
 */
void write_byte(unsigned int byte) {
    unsigned int offset = get_rom_index();

    if (ines.trn == 1) {
        if (pass == 2) {
            trainer[offset_trainer] = byte;
        }

        offset_trainer++;

        return;
    }

    if (pass == 2) {
        rom[offset] = byte;
    }

    if (is_segment_prg() == TRUE) {
        prg_offsets[prg_index]++;
    }

    if (is_segment_chr() == TRUE) {
        chr_offsets[chr_index]++;
    }

    if ((int)offset + 1 > offset_max) {
        offset_max = (int)offset + 1;
    }
}

/**
 * Add symbol
 * @param {char *} name - Symbol name
 * @param {unsigned int} value - Symbol value
 * @param {unsigned int} type - Symbol type
 */
void add_symbol(char *name, unsigned int value, unsigned int type) {
    if (pass == 1) {
        symbols[symbol_index].name = name;
        symbols[symbol_index].value = value;
        symbols[symbol_index++].type = type;
    }
}

/**
 * Get symbol
 * @param {char *} name - Symbol name
 * @return {int} Symbol ID
 */
int get_symbol(char *name) {
    int symbol_id = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = symbol_index; i < l; i++) {
        if (symbol_id == -1 && strcmp(symbols[i].name, name) == 0) {
            symbol_id = (int)i;
        }
    }

    if (symbol_id == -1) {
        yyerror("Invalid symbol `%s`", name);
    }

    return symbol_id;
}

/**
 * Add constant
 * @param {char *} name - Constant name
 * @param {int} value - Constant value
 */
void add_constant(char *name, unsigned int value) {
    add_symbol(name, value, SYMBOL_CONSTANT);
}

/**
 * Add label
 * @param {char *} name - Label name
 */
void add_label(char *name) {
    unsigned int offset = get_address_offset();

    add_symbol(name, offset, SYMBOL_LABEL);
}
