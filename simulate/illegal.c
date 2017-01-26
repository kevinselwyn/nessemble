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
    // TODO: Undocumented
}

void do_arr(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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

void do_dcp(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_dop(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_isc(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
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

void do_rla(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_rra(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_slo(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sre(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sxa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sya(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_top(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_xaa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_xas(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}
