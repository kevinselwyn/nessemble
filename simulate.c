#include <string.h>
#include <stdlib.h>
#include "nessemble.h"

// https://github.com/bfirsh/jsnes/blob/master/source/cpu.js

struct flgs {
    int negative, overflow, brk, decimal, ineterrupt, zero, carry;
};

struct regs {
    int a, x, y;
    int pc, sp;
    struct flgs flags;
};

struct regs registers = { 0, 0, 0, 0x8000, 0xFF, { 0, 0, 0, 0, 0, 0, 0 } };

char *rom_data = NULL;

int simulate(char *input) {
    int rc = RETURN_OK, i = 0, l = 0, header = 0;
    int inesprg = 1, ineschr = 1;
    int opcode_index = -1, arg0 = 0, arg1 = 0, length = 0;
    size_t insize = 0;
    char *indata = NULL;

    insize = load_file(&indata, input);

    if (!insize) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    // nes test
    if (strncmp(indata, "NES", 3) == 0) {
        header = 1;
    }

    // load rom
    rom_data = (char *)malloc(sizeof(char) * 0x10000);

    if (!rom_data) {
        fprintf(stderr, "Memory error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = 0x10000; i < l; i++) {
        rom_data[i] = 0xFF;
    }

    // load banks
    if (header) {
        inesprg = (int)indata[4];
        ineschr = (int)indata[5];

        if (inesprg == 1) {
            registers.pc = 0xC000;
            memcpy(rom_data+registers.pc, indata+0x10, 0x4000);
        } else {
            registers.pc = 0x8000;
            memcpy(rom_data+registers.pc, indata+0x10, 0x8000);
        }
    } else {
        registers.pc = 0x8000;
        memcpy(rom_data+registers.pc, indata, insize);
    }

    // simulate
    while (1) {
        opcode_index = (int)rom_data[registers.pc] & 0xFF;
        length = opcodes[opcode_index].length;

        printf(" PC  AC XR YR SP NV-BDIZC\n");
        printf("%04X ", registers.pc);
        printf("%02X ", registers.a);
        printf("%02X ", registers.x);
        printf("%02X ", registers.y);
        printf("%02X ", registers.sp);

        printf("%d", registers.flags.negative & 1);
        printf("%d0", registers.flags.overflow & 1);
        printf("%d", registers.flags.brk & 1);
        printf("%d", registers.flags.decimal & 1);
        printf("%d", registers.flags.ineterrupt & 1);
        printf("%d", registers.flags.zero & 1);
        printf("%d", registers.flags.carry & 1);

        printf("\n\n");

        if (length == 1) {
            (*opcodes[opcode_index].func)(opcode_index, 0);
        } else {
            arg0 = (int)rom_data[registers.pc+1] & 0xFF;

            if (length == 2) {
                (*opcodes[opcode_index].func)(opcode_index, arg0);
            } else if (length == 3) {
                arg1 = (int)rom_data[registers.pc+2] & 0xFF;
                (*opcodes[opcode_index].func)(opcode_index, (arg1 << 8) | arg0);
            }
        }

        if (registers.pc == -1) {
            break;
        }

        getchar();
    }

cleanup:
    if (rom_data) {
        free(rom_data);
    }

    return rc;
}

int do_aac(int opcode_index, int value) {

}

int do_aax(int opcode_index, int value) {

}

int do_adc(int opcode_index, int value) {

}

int do_and(int opcode_index, int value) {

}

int do_arr(int opcode_index, int value) {

}

int do_asl(int opcode_index, int value) {

}

int do_asr(int opcode_index, int value) {

}

int do_atx(int opcode_index, int value) {

}

int do_axa(int opcode_index, int value) {

}

int do_axs(int opcode_index, int value) {

}

int do_bcc(int opcode_index, int value) {

}

int do_bcs(int opcode_index, int value) {

}

int do_beq(int opcode_index, int value) {

}

int do_bit(int opcode_index, int value) {

}

int do_bmi(int opcode_index, int value) {

}

int do_bne(int opcode_index, int value) {

}

int do_bpl(int opcode_index, int value) {

}

int do_brk(int opcode_index, int value) {

}

int do_bvc(int opcode_index, int value) {

}

int do_bvs(int opcode_index, int value) {

}

int do_clc(int opcode_index, int value) {

}

int do_cld(int opcode_index, int value) {

}

int do_cli(int opcode_index, int value) {

}

int do_clv(int opcode_index, int value) {

}

int do_cmp(int opcode_index, int value) {

}

int do_cpx(int opcode_index, int value) {

}

int do_cpy(int opcode_index, int value) {

}

int do_dcp(int opcode_index, int value) {

}

int do_dec(int opcode_index, int value) {

}

int do_dex(int opcode_index, int value) {

}

int do_dey(int opcode_index, int value) {

}

int do_dop(int opcode_index, int value) {

}

int do_eor(int opcode_index, int value) {

}

int do_inc(int opcode_index, int value) {

}

int do_inx(int opcode_index, int value) {

}

int do_iny(int opcode_index, int value) {

}

int do_isc(int opcode_index, int value) {

}

int do_jmp(int opcode_index, int value) {

}

int do_jsr(int opcode_index, int value) {

}

int do_kil(int opcode_index, int value) {

}

int do_lar(int opcode_index, int value) {

}

int do_lax(int opcode_index, int value) {

}

int do_lda(int opcode_index, int value) {
    int length = 0, mode = 0;

    length = opcodes[opcode_index].length;
    mode = opcodes[opcode_index].mode;

    if ((mode & MODE_IMMEDIATE) != 0) {
        registers.a = value;
    } else {
        registers.a = (int)rom_data[value];
    }

    registers.flags.negative = (registers.a >> 7) & 1;
    registers.flags.zero = registers.a == 0 ? 1 : 0;
    registers.pc += length;
}

int do_ldx(int opcode_index, int value) {

}

int do_ldy(int opcode_index, int value) {

}

int do_lsr(int opcode_index, int value) {

}

int do_nop(int opcode_index, int value) {

}

int do_ora(int opcode_index, int value) {

}

int do_pha(int opcode_index, int value) {

}

int do_php(int opcode_index, int value) {

}

int do_pla(int opcode_index, int value) {

}

int do_plp(int opcode_index, int value) {

}

int do_rla(int opcode_index, int value) {

}

int do_rol(int opcode_index, int value) {

}

int do_ror(int opcode_index, int value) {

}

int do_rra(int opcode_index, int value) {

}

int do_rti(int opcode_index, int value) {

}

int do_rts(int opcode_index, int value) {

}

int do_sbc(int opcode_index, int value) {

}

int do_sec(int opcode_index, int value) {

}

int do_sed(int opcode_index, int value) {

}

int do_sei(int opcode_index, int value) {

}

int do_slo(int opcode_index, int value) {

}

int do_sre(int opcode_index, int value) {

}

int do_sta(int opcode_index, int value) {
    int length = 0;

    length = opcodes[opcode_index].length;

    rom_data[registers.a] = (char)value;

    registers.pc += length;
}

int do_stx(int opcode_index, int value) {

}

int do_sty(int opcode_index, int value) {

}

int do_sxa(int opcode_index, int value) {

}

int do_sya(int opcode_index, int value) {

}

int do_tax(int opcode_index, int value) {

}

int do_tay(int opcode_index, int value) {

}

int do_top(int opcode_index, int value) {

}

int do_tsx(int opcode_index, int value) {

}

int do_txa(int opcode_index, int value) {

}

int do_txs(int opcode_index, int value) {

}

int do_tya(int opcode_index, int value) {

}

int do_xaa(int opcode_index, int value) {

}

int do_xas(int opcode_index, int value) {

}
