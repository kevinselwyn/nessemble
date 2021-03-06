#include "../nessemble.h"

void do_branch(unsigned int opcode_index, unsigned int value, unsigned int flag, unsigned int boolean) {
    unsigned int address = 0, tmp = 0, timing = 0;

    tmp = (unsigned int)get_register(REGISTER_PC);
    timing = opcodes[opcode_index].timing;

    if (value >= 0x80) {
        address = tmp - (0xFF - value - 1);
    } else {
        address = tmp + value;
    }

    if (get_flag(flag) == boolean) {
        if ((tmp & 0xFF00) != (address & 0xFF00)) {
            timing += 2;
        } else {
            timing += 1;
        }

        set_register(REGISTER_PC, address);
    } else {
        inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    }

    inc_cycles(timing);
}

void do_adc(unsigned int opcode_index, unsigned int value) {
    int tmp = 0;
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = (int)(get_register(REGISTER_A) + value + get_flag(FLG_CARRY));
        set_flag(FLG_NEGATIVE, ((unsigned int)tmp >> 7) & 1);
        set_flag(FLG_OVERFLOW, (unsigned int)((((get_register(REGISTER_A) ^ tmp) & 0x80) != 0 && ((get_register(REGISTER_A) ^ value) & 0x80) != 0) ? TRUE : FALSE));
    } else {
        address = get_address(opcode_index, value);
        tmp = (int)(get_register(REGISTER_A) + get_byte(address) + get_flag(FLG_CARRY));
        set_flag(FLG_NEGATIVE, ((unsigned int)tmp >> 7) & 1);
        set_flag(FLG_OVERFLOW, (unsigned int)((((get_register(REGISTER_A) ^ tmp) & 0x80) != 0 && ((get_register(REGISTER_A) ^ get_byte(address)) & 0x80) != 0) ? TRUE : FALSE));
    }

    set_register(REGISTER_A, (unsigned int)(tmp & 0xFF));
    set_flag(FLG_ZERO, (unsigned int)((tmp & 0xFF) == 0 ? TRUE : FALSE));
    set_flag(FLG_CARRY, (unsigned int)(tmp > 0xFF ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_and(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    tmp = (tmp & get_register(REGISTER_A)) & 0xFF;

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_register(REGISTER_A, tmp);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_asl(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_ACCUMULATOR) {
        tmp = get_register(REGISTER_A) & 0xFF;
        set_flag(FLG_CARRY, (tmp << 7) & 1);
        tmp = (tmp << 1) & 0xFF;
        set_register(REGISTER_A, tmp);
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
        set_flag(FLG_CARRY, (tmp << 7) & 1);
        tmp = (tmp << 1) & 0xFF;
        set_byte(address, tmp);
    }

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_bcc(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_CARRY, FALSE);
}

void do_bcs(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_CARRY, TRUE);
}

void do_beq(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_ZERO, TRUE);
}

void do_bit(unsigned int opcode_index, unsigned int value) {
    unsigned int address = 0, tmp = 0;

    address = get_address(opcode_index, value);
    tmp = get_byte(address);

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_OVERFLOW, (tmp >> 6) & 1);

    tmp &= get_register(REGISTER_A);

    set_flag(FLG_ZERO, (tmp == 0 ? TRUE : FALSE));

    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
}

void do_bmi(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_NEGATIVE, TRUE);
}

void do_bne(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_ZERO, FALSE);
}

void do_bpl(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_NEGATIVE, FALSE);
}

void do_brk(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    inc_register(REGISTER_PC, 2);
    stack_push((get_register(REGISTER_PC) >> 8) & 0xFF);
    stack_push(get_register(REGISTER_PC) & 0xFF);
    set_flag(FLG_BREAK, TRUE);

    tmp |= get_flag(FLG_CARRY);
    tmp |= get_flag(FLG_ZERO) << 1;
    tmp |= get_flag(FLG_INTERRUPT) << 2;
    tmp |= get_flag(FLG_DECIMAL) << 3;
    tmp |= get_flag(FLG_BREAK) << 4;
    tmp |= get_flag(FLG_OVERFLOW) << 6;
    tmp |= get_flag(FLG_NEGATIVE) << 7;

    stack_push(tmp);

    set_flag(FLG_INTERRUPT, TRUE);

    tmp = get_byte(0xFFFE);
    tmp |= get_byte(0xFFFF) << 8;

    set_register(REGISTER_PC, tmp);

    UNUSED(opcode_index);
    UNUSED(value);
}

void do_bvc(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_OVERFLOW, FALSE);
}

void do_bvs(unsigned int opcode_index, unsigned int value) {
    do_branch(opcode_index, value, FLG_OVERFLOW, TRUE);
}

void do_clc(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_CARRY, FALSE);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);

    UNUSED(value);
}

void do_cld(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_DECIMAL, FALSE);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);

    UNUSED(value);
}

void do_cli(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_INTERRUPT, FALSE);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);

    UNUSED(value);
}

void do_clv(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_OVERFLOW, FALSE);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);

    UNUSED(value);
}

void do_cmp(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;
    int tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = (int)value;
    } else {
        address = get_address(opcode_index, value);
        tmp = (int)get_byte(address);
    }

    tmp = get_register(REGISTER_A) - tmp;

    set_flag(FLG_CARRY, (unsigned int)(tmp >= 0 ? TRUE : FALSE));
    set_flag(FLG_NEGATIVE, (unsigned int)((tmp >> 7) & 1));
    set_flag(FLG_ZERO, (unsigned int)((tmp & 0xFF) == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_cpx(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;
    int tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = (int)value;
    } else {
        address = get_address(opcode_index, value);
        tmp = (int)get_byte(address);
    }

    tmp = get_register(REGISTER_X) - tmp;

    set_flag(FLG_CARRY, (unsigned int)(tmp >= 0 ? TRUE : FALSE));
    set_flag(FLG_NEGATIVE, (unsigned int)((tmp >> 7) & 1));
    set_flag(FLG_ZERO, (unsigned int)((tmp & 0xFF) == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_cpy(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;
    int tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = (int)value;
    } else {
        address = get_address(opcode_index, value);
        tmp = (int)get_byte(address);
    }

    tmp = get_register(REGISTER_Y) - tmp;

    set_flag(FLG_CARRY, (unsigned int)(tmp >= 0 ? TRUE : FALSE));
    set_flag(FLG_NEGATIVE, (unsigned int)((tmp >> 7) & 1));
    set_flag(FLG_ZERO, (unsigned int)((tmp & 0xFF) == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dec(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value), tmp = 0;

    tmp = (get_byte(address) - 1) & 0xFF;

    set_byte(address, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dex(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_X) - 1) & 0xFF;

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_dey(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_Y) - 1) & 0xFF;

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_eor(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    tmp = (tmp ^ get_register(REGISTER_A)) & 0xFF;

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_register(REGISTER_A, tmp);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_inc(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value), tmp = 0;

    tmp = (get_byte(address) + 1) & 0xFF;

    set_byte(address, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_inx(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_X) + 1) & 0xFF;

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_iny(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_Y) + 1) & 0xFF;

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_jmp(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    set_register(REGISTER_PC, address);
}

void do_jsr(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value), tmp = 0;

    tmp = get_register(REGISTER_PC);

    stack_push((tmp >> 8) & 0xFF);
    stack_push(tmp & 0xFF);
    set_register(REGISTER_PC, address);

    inc_cycles(opcodes[opcode_index].timing);
}

void do_lda(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_ldx(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_ldy(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_lsr(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_ACCUMULATOR) {
        tmp = get_register(REGISTER_A) & 0xFF;
        set_flag(FLG_CARRY, tmp & 1);
        tmp = (tmp >> 1) & 0xFF;
        set_register(REGISTER_A, tmp);
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
        set_flag(FLG_CARRY, tmp & 1);
        tmp = (tmp >> 1) & 0xFF;
        set_byte(address, tmp);
    }

    set_flag(FLG_NEGATIVE, FALSE);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_nop(unsigned int opcode_index, unsigned int value) {
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_ora(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = value;
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
    }

    tmp = (tmp | get_register(REGISTER_A)) & 0xFF;

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));
    set_register(REGISTER_A, tmp);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_pha(unsigned int opcode_index, unsigned int value) {
    stack_push(get_register(REGISTER_A));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_php(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    set_flag(FLG_BREAK, TRUE);

    tmp |= get_flag(FLG_CARRY);
    tmp |= get_flag(FLG_ZERO) << 1;
    tmp |= get_flag(FLG_INTERRUPT) << 2;
    tmp |= get_flag(FLG_DECIMAL) << 3;
    tmp |= get_flag(FLG_BREAK) << 4;
    tmp |= get_flag(FLG_OVERFLOW) << 6;
    tmp |= get_flag(FLG_NEGATIVE) << 7;

    stack_push(tmp);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_pla(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull();

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_plp(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull();

    set_flag(FLG_CARRY, tmp & 1);
    set_flag(FLG_ZERO, (tmp >> 1) & 1);
    set_flag(FLG_INTERRUPT, (tmp >> 2) & 1);
    set_flag(FLG_DECIMAL, (tmp >> 3) & 1);
    set_flag(FLG_BREAK, (tmp >> 4) & 1);
    set_flag(FLG_OVERFLOW, (tmp >> 6) & 1);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_rol(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0, add = 0;

    if (mode == MODE_ACCUMULATOR) {
        tmp = get_register(REGISTER_A);
        add = get_flag(FLG_CARRY);
        set_flag(FLG_CARRY, (tmp >> 7) & 1);
        tmp = ((tmp << 1) & 0xFF) + add;
        set_register(REGISTER_A, tmp);
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address);
        add = get_flag(FLG_CARRY);
        set_flag(FLG_CARRY, (tmp >> 7) & 1);
        tmp = ((tmp << 1) & 0xFF) + add;
        set_byte(address, tmp);
    }

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_ror(unsigned int opcode_index, unsigned int value) {
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0, tmp = 0, add = 0;

    if (mode == MODE_ACCUMULATOR) {
        add = get_flag(FLG_CARRY) << 7;
        set_flag(FLG_CARRY, get_register(REGISTER_A) & 1);
        tmp = (get_register(REGISTER_A) >> 1) + add;
        set_register(REGISTER_A, tmp);
    } else {
        address = get_address(opcode_index, value);
        tmp = get_byte(address) & 0xFF;
        add = get_flag(FLG_CARRY) << 7;
        set_flag(FLG_CARRY, tmp & 1);
        tmp = (tmp >> 1) + add;
        set_byte(address, tmp);
    }

    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_rti(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull();

    set_flag(FLG_CARRY, tmp & 1);
    set_flag(FLG_ZERO, (tmp >> 1) & 1);
    set_flag(FLG_INTERRUPT, (tmp >> 2) & 1);
    set_flag(FLG_DECIMAL, (tmp >> 3) & 1);
    set_flag(FLG_BREAK, (tmp >> 4) & 1);
    set_flag(FLG_OVERFLOW, (tmp >> 6) & 1);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);

    tmp = (stack_pull() | (stack_pull() << 8)) & 0xFFFF;

    set_register(REGISTER_PC, tmp);

    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_rts(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull() | (stack_pull() << 8);

    set_register(REGISTER_PC, tmp);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_sbc(unsigned int opcode_index, unsigned int value) {
    int tmp = 0;
    unsigned int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;

    if (mode == MODE_IMMEDIATE) {
        tmp = (int)(get_register(REGISTER_A) - value - (1 - get_flag(FLG_CARRY)));
        set_flag(FLG_NEGATIVE, ((unsigned int)tmp >> 7) & 1);
        set_flag(FLG_OVERFLOW, (unsigned int)((((get_register(REGISTER_A) ^ tmp) & 0x80) != 0 && ((get_register(REGISTER_A) ^ value) & 0x80) != 0) ? TRUE : FALSE));
    } else {
        address = get_address(opcode_index, value);
        tmp = (int)(get_register(REGISTER_A) - get_byte(address) - (1 - get_flag(FLG_CARRY)));
        set_flag(FLG_NEGATIVE, ((unsigned int)tmp >> 7) & 1);
        set_flag(FLG_OVERFLOW, (unsigned int)((((get_register(REGISTER_A) ^ tmp) & 0x80) != 0 && ((get_register(REGISTER_A) ^ get_byte(address)) & 0x80) != 0) ? TRUE : FALSE));
    }

    set_register(REGISTER_A, (unsigned int)(tmp & 0xFF));
    set_flag(FLG_ZERO, (unsigned int)((tmp & 0xFF) == 0 ? TRUE : FALSE));
    set_flag(FLG_CARRY, (unsigned int)(tmp < 0 ? FALSE : TRUE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sec(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_CARRY, TRUE);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_sed(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_DECIMAL, TRUE);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_sei(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_INTERRUPT, TRUE);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_sta(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    set_byte(address, get_register(REGISTER_A));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_stx(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    set_byte(address, get_register(REGISTER_X));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sty(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    set_byte(address, get_register(REGISTER_Y));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_tax(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_A);

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_tay(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_A);

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_tsx(unsigned int opcode_index, unsigned int value) {
    set_register(REGISTER_X, get_register(REGISTER_SP));
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_txa(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_X);

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_txs(unsigned int opcode_index, unsigned int value) {
    set_register(REGISTER_SP, get_register(REGISTER_X));
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}

void do_tya(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_Y);

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);

    UNUSED(value);
}
