#include <string.h>
#include "nessemble.h"
#include "opcodes.h"

#define ERROR_OPCODE   "Unknown opcode `%s`"
#define ERROR_REGISTER "Unknown register `%c`"

/**
 * Get opcode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} mode - Addressing mode
 */
int get_opcode(char *mnemonic, int mode) {
    int i = 0, l = 0, opcode_index = -1;

    for (i = 0, l = OPCODE_COUNT; i < l; i++) {
        if (opcode_index == -1 && opcodes[i].mode == mode && strcmp(opcodes[i].mnemonic, mnemonic) == 0) {
            if (is_flag_undocumented()) {
                if ((opcodes[i].meta & META_UNDOCUMENTED) != 0) {
                    opcode_index = i;
                }
            } else {
                if ((opcodes[i].meta & META_UNDOCUMENTED) == 0) {
                    opcode_index = i;
                }
            }
        }
    }

    return opcode_index;
}

/**
 * Assemble absolute addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_absolute(char *mnemonic, int address) {
    int opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE);

    if (opcode_index == -1) {
        opcode_index = get_opcode(mnemonic, MODE_RELATIVE);

        if (opcode_index != -1) {
            assemble_relative(mnemonic, address);
            return;
        }
    }

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble absolute_x/absolute_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_absolute_xy(char *mnemonic, int address, char reg) {
    int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_Y);
    } else {
        yyerror(ERROR_REGISTER, reg);
    }

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble accumulator addressing mode
 * @param {char *} mnemonic - Mnemonic
 */
void assemble_accumulator(char *mnemonic) {
    int opcode_index = get_opcode(mnemonic, MODE_ACCUMULATOR);

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
}

/**
 * Assemble implied addressing mode
 * @param {char *} mnemonic - Mnemonic
 */
void assemble_implied(char *mnemonic) {
    int opcode_index = get_opcode(mnemonic, MODE_IMPLIED);

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
}

/**
 * Assemble immediate addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} value - Value
 */
void assemble_immediate(char *mnemonic, int value) {
    int opcode_index = get_opcode(mnemonic, MODE_IMMEDIATE);

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(value & 0xFF);
}

/**
 * Assemble indirect addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_indirect(char *mnemonic, int address) {
    int opcode_index = get_opcode(mnemonic, MODE_INDIRECT);

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble indirect_x/indirect_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_indirect_xy(char *mnemonic, int address, char reg) {
    int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_Y);
    } else {
        yyerror(ERROR_REGISTER, reg);
    }

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
}

/**
 * Assemble relative addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_relative(char *mnemonic, int address) {
    int opcode_id = get_opcode(mnemonic, MODE_RELATIVE);
    int offset = get_address_offset() + 1;

    if (offset > address) {
        address = 0xFF - (offset - address);
    } else {
        address = address - offset - 1;
    }

    // TODO: throw error if jump is too large
    address &= 0xFF;

    write_byte((unsigned int)opcode_id);
    write_byte(address);
}

/**
 * Assemble zeropage addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_zeropage(char *mnemonic, int address) {
    int opcode_id = get_opcode(mnemonic, MODE_ZEROPAGE);

    write_byte((unsigned int)opcode_id);
    write_byte(address & 0XFF);
}

/**
 * Assemble zeropage_x/zeropage_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_zeropage_xy(char *mnemonic, int address, char reg) {
    int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_Y);
    } else {
        yyerror(ERROR_REGISTER, reg);
    }

    if (opcode_index == -1) {
        yyerror(ERROR_OPCODE, mnemonic);
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
}
