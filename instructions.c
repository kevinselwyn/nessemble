#include <string.h>
#include "nessemble.h"

/**
 * Check if mnemonic exists
 * @param {char *} mnemonic - Mnemonic
 * @return {unsigned int} True if mnemonic exists, false if not
 */
static unsigned int mnemonic_exists(char *mnemonic) {
    unsigned int exists = FALSE, i = 0, l = 0;

    for (i = 0, l = OPCODE_COUNT; i < l; i++) {
        if (strcmp(opcodes[i].mnemonic, mnemonic) == 0) {
            exists = TRUE;
        }
    }

    if (exists == TRUE) {
        error("Invalid addressing mode");
    } else {
        error("Unknown opcode `%s`", mnemonic);
    }

    return exists;
}

/**
 * Check if register exists
 * @param {char} reg - Register
 * @param {char *} regs - Allowed registers
 * @return {unsigned int} True if register exists, false if not
 */
static unsigned int register_exists(char reg, char *regs) {
    unsigned int exists = FALSE, i = 0, l = 0;
    size_t length = 0;

    length = strlen(regs);

    for (i = 0, l = (unsigned int)length; i < l; i++) {
        if (reg == regs[i]) {
            exists = TRUE;
        }
    }

    if (exists != TRUE) {
        error("Unknown register `%c`", reg);
    }

    return exists;
}

/**
 * Get opcode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} mode - Addressing mode
 */
int get_opcode(char *mnemonic, unsigned int mode) {
    int opcode_index = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = OPCODE_COUNT; i < l; i++) {
        if (opcode_index == -1 && opcodes[i].mode == mode && strcmp(opcodes[i].mnemonic, mnemonic) == 0) {
            if (is_flag_undocumented() == TRUE) {
                if ((opcodes[i].meta & META_UNDOCUMENTED) != 0) {
                    opcode_index = (int)i;
                }
            } else {
                if ((opcodes[i].meta & META_UNDOCUMENTED) == 0) {
                    opcode_index = (int)i;
                }
            }
        }
    }

    return opcode_index;
}

/**
 * Assemble absolute addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 */
void assemble_absolute(char *mnemonic, unsigned int address) {
    int opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE);

    if (opcode_index == -1) {
        opcode_index = get_opcode(mnemonic, MODE_RELATIVE);

        if (opcode_index != -1) {
            assemble_relative(mnemonic, address);
            return;
        }
    }

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble absolute_x/absolute_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 * @param {char} reg - Register
 */
void assemble_absolute_xy(char *mnemonic, unsigned int address, char reg) {
    int opcode_index = -1;

    if (register_exists(reg, "XY") != TRUE) {
        return;
    }

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_Y);
    }

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble accumulator addressing mode
 * @param {char *} mnemonic - Mnemonic
 */
void assemble_accumulator(char *mnemonic, char reg) {
    int opcode_index = get_opcode(mnemonic, MODE_ACCUMULATOR);

    if (register_exists(reg, "A") != TRUE) {
        return;
    }

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
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
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
}

/**
 * Assemble immediate addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} value - Value
 */
void assemble_immediate(char *mnemonic, unsigned int value) {
    int opcode_index = get_opcode(mnemonic, MODE_IMMEDIATE);

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(value & 0xFF);
}

/**
 * Assemble indirect addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 */
void assemble_indirect(char *mnemonic, unsigned int address) {
    int opcode_index = get_opcode(mnemonic, MODE_INDIRECT);

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble indirect_x/indirect_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 * @param {char} reg - Register
 */
void assemble_indirect_xy(char *mnemonic, unsigned int address, char reg) {
    int opcode_index = -1;

    if (register_exists(reg, "XY") != TRUE) {
        return;
    }

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_Y);
    }

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
}

/**
 * Assemble relative addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 */
void assemble_relative(char *mnemonic, unsigned int address) {
    int opcode_id = get_opcode(mnemonic, MODE_RELATIVE);
    unsigned int offset = get_address_offset() + 1;

    if (offset > address) {
        address = 0xFF - (offset - address);

        if (pass == 2) {
            if (address <= 0x7F) {
                error("Branch address out of range");
            }
        }
    } else {
        address = address - offset - 1;

        if (pass == 2) {
            if (address >= 0x80) {
                error("Branch address out of range");
            }
        }
    }

    address &= 0xFF;

    write_byte((unsigned int)opcode_id);
    write_byte(address);
}

/**
 * Assemble zeropage addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {unsigned int} address - Address
 */
void assemble_zeropage(char *mnemonic, unsigned int address) {
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
void assemble_zeropage_xy(char *mnemonic, unsigned int address, char reg) {
    int opcode_index = -1;

    if (register_exists(reg, "XY") != TRUE) {
        return;
    }

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_Y);
    }

    if (opcode_index == -1) {
        if (mnemonic_exists(mnemonic) != TRUE) {
            return;
        }
    }

    write_byte((unsigned int)opcode_index);
    write_byte(address & 0xFF);
}
