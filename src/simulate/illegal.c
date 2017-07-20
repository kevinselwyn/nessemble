#include "../nessemble.h"

void do_aac(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = (value & get_register(REGISTER_A)) & 0xFF;

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_aax(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = (get_register(REGISTER_X) & get_register(REGISTER_A)) & 0xFF;
    address = get_address(opcode_index, value);

    set_byte(address, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_arr(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = ((value & get_register(REGISTER_A)) >> 1) & 0xFF;

    set_register(REGISTER_A, tmp);

    if (((tmp >> 5) & 0x03) == 0x03) {
        set_flag(FLG_CARRY, TRUE);
        set_flag(FLG_OVERFLOW, FALSE);
    } else if (((tmp >> 5) & 0x03) == 0) {
        set_flag(FLG_CARRY, FALSE);
        set_flag(FLG_OVERFLOW, FALSE);
    } else if (((tmp >> 5) & 1) == 1) {
        set_flag(FLG_CARRY, FALSE);
        set_flag(FLG_OVERFLOW, TRUE);
    } else if (((tmp >> 6) & 1) == 1) {
        set_flag(FLG_CARRY, TRUE);
        set_flag(FLG_OVERFLOW, TRUE);
    }

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_asr(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = ((value & get_register(REGISTER_A)) >> 1);

    set_register(REGISTER_A, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_flag(FLG_CARRY, FALSE);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_atx(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = value & get_register(REGISTER_A);

    set_register(REGISTER_X, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_axa(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = get_register(REGISTER_X) & get_register(REGISTER_A);
    tmp &= 0x07;

    address = get_address(opcode_index, value);
    set_byte(address, tmp);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_axs(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    tmp = get_register(REGISTER_X) & get_register(REGISTER_A);
    tmp = (tmp - value) & 0xFF;

    set_register(REGISTER_X, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dcp(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    if (is_flag_undocumented() == FALSE) {
        inc_register(REGISTER_PC, 1);
        return;
    }

    address = get_address(opcode_index, value);
    set_byte(address, (get_byte(address) - 1) & 0xFF);

    set_flag(FLG_CARRY, (tmp >> 7) & 1);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dop(unsigned int opcode_index, unsigned int value) {
    UNUSED(opcode_index);
    UNUSED(value);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_isc(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp = (tmp + 1) & 0xFF;
    set_byte(address, tmp);

    tmp = (get_register(REGISTER_A) - tmp) & 0xFF;
    set_register(REGISTER_A, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_flag(FLG_OVERFLOW, (tmp >> 6) & 1);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_kil(unsigned int opcode_index, unsigned int value) {
    UNUSED(opcode_index);
    UNUSED(value);

    set_register(REGISTER_PC, -1);
}

void do_lar(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp &= get_register(REGISTER_SP);

    set_register(REGISTER_A, tmp);
    set_register(REGISTER_X, tmp);
    set_register(REGISTER_SP, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_lax(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    set_register(REGISTER_A, tmp);
    set_register(REGISTER_X, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_rla(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp <<= 1;
    tmp &= get_register(REGISTER_A);
    tmp &= 0xFF;

    set_byte(address, tmp);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_rra(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp >>= 1;

    set_register(REGISTER_A, (tmp + get_register(REGISTER_A)) & 0xFF);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_flag(FLG_OVERFLOW, (tmp >> 6) & 1);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_slo(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp <<= 1;

    set_register(REGISTER_A, (tmp | get_register(REGISTER_A)) & 0xFF);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sre(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0, address = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    tmp >>= 1;

    set_register(REGISTER_A, (tmp ^ get_register(REGISTER_A)) & 0xFF);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_CARRY, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sxa(unsigned int opcode_index, unsigned int value) {
    /* TODO: Undocumented */
}

void do_sya(unsigned int opcode_index, unsigned int value) {
    /* TODO: Undocumented */
}

void do_top(unsigned int opcode_index, unsigned int value) {
    /* TODO: Undocumented */
}

void do_xaa(unsigned int opcode_index, unsigned int value) {
    /* TODO: Undocumented */
}

void do_xas(unsigned int opcode_index, unsigned int value) {
    /* TODO: Undocumented */
}
