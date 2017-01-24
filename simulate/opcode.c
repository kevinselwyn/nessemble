#include "../nessemble.h"

void do_aac(unsigned int opcode_index, unsigned int value) {

}

void do_aax(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_adc(unsigned int opcode_index, unsigned int value) {

}

void do_and(unsigned int opcode_index, unsigned int value) {

}

void do_arr(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_asl(unsigned int opcode_index, unsigned int value) {

}

void do_asr(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_atx(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_axa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_axs(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_bcc(unsigned int opcode_index, unsigned int value) {

}

void do_bcs(unsigned int opcode_index, unsigned int value) {

}

void do_beq(unsigned int opcode_index, unsigned int value) {
    unsigned int address = 0, tmp = 0, timing = 0;

    tmp = (unsigned int)get_register(REGISTER_PC);
    timing = opcodes[opcode_index].timing;

    if (value >= 0x80) {
        address = tmp - (0xFF - value - 1);
    } else {
        address = tmp + value;
    }

    if (get_flag(FLG_ZERO) != 0) {
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

void do_bit(unsigned int opcode_index, unsigned int value) {

}

void do_bmi(unsigned int opcode_index, unsigned int value) {

}

void do_bne(unsigned int opcode_index, unsigned int value) {
    unsigned int address = 0, tmp = 0, timing = 0;

    tmp = (unsigned int)get_register(REGISTER_PC);
    timing = opcodes[opcode_index].timing;

    if (value >= 0x80) {
        address = tmp - (0xFF - value - 1);
    } else {
        address = tmp + value;
    }

    if (get_flag(FLG_ZERO) == 0) {
        if ((tmp & 0xFF00) != (address& 0xFF00)) {
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

void do_bpl(unsigned int opcode_index, unsigned int value) {

}

void do_brk(unsigned int opcode_index, unsigned int value) {

}

void do_bvc(unsigned int opcode_index, unsigned int value) {

}

void do_bvs(unsigned int opcode_index, unsigned int value) {

}

void do_clc(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_CARRY, 0);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
}

void do_cld(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_DECIMAL, 0);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
}

void do_cli(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_INTERRUPT, 0);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
}

void do_clv(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_OVERFLOW, 0);
    inc_cycles(opcodes[opcode_index].timing);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
}

void do_cmp(unsigned int opcode_index, unsigned int value) {

}

void do_cpx(unsigned int opcode_index, unsigned int value) {

}

void do_cpy(unsigned int opcode_index, unsigned int value) {

}

void do_dcp(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_dec(unsigned int opcode_index, unsigned int value) {

}

void do_dex(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_X) - 1) & 0xFF;

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dey(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_Y) - 1) & 0xFF;

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_dop(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_eor(unsigned int opcode_index, unsigned int value) {

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
}

void do_iny(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = (get_register(REGISTER_Y) + 1) & 0xFF;

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_isc(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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

void do_kil(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_lar(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_lax(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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

    set_flag(FLG_NEGATIVE, 0);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_nop(unsigned int opcode_index, unsigned int value) {
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
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

    tmp = (tmp & get_register(REGISTER_A)) & 0xFF;

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
}

void do_php(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    set_flag(FLG_BREAK, 1);

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
}

void do_pla(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull();

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
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
}

void do_rla(unsigned int opcode_index, unsigned int value) {

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

void do_rra(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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
}

void do_rts(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = stack_pull() | (stack_pull() << 8);

    set_register(REGISTER_PC, tmp);
    inc_cycles(opcodes[opcode_index].timing);
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
    set_flag(FLG_CARRY, 1);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sed(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_DECIMAL, 1);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_sei(unsigned int opcode_index, unsigned int value) {
    set_flag(FLG_INTERRUPT, 1);
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_slo(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sre(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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

void do_sxa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sya(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_tax(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_A);

    set_register(REGISTER_X, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_tay(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_A);

    set_register(REGISTER_Y, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_top(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_tsx(unsigned int opcode_index, unsigned int value) {
    set_register(REGISTER_X, get_register(REGISTER_SP));
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_txa(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_X);

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_txs(unsigned int opcode_index, unsigned int value) {
    set_register(REGISTER_SP, get_register(REGISTER_X));
    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_tya(unsigned int opcode_index, unsigned int value) {
    unsigned int tmp = 0;

    tmp = get_register(REGISTER_Y);

    set_register(REGISTER_A, tmp);
    set_flag(FLG_NEGATIVE, (tmp >> 7) & 1);
    set_flag(FLG_ZERO, (unsigned int)(tmp == 0 ? TRUE : FALSE));

    inc_register(REGISTER_PC, (int)opcodes[opcode_index].length);
    inc_cycles(opcodes[opcode_index].timing);
}

void do_xaa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_xas(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}
