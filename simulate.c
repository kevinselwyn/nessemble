#include <stdlib.h>
#include <string.h>
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

#define BUF_SIZE 256

char buffer[BUF_SIZE];

// utils
void print_registers();
void print_instruction();
int step();
void print_memory(char *input);
void print_goto(char *input);

/**
 * Simulate 6502
 * @param {char *} input - Input filename
 */
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
        if (registers.pc == -1) {
            break;
        }

        printf("nessemble> ");

        fgets(buffer, BUF_SIZE, stdin);

        if (strncmp(buffer, ".registers", 10) == 0) {
            print_registers();
        }

        if (strncmp(buffer, ".instruction", 12) == 0) {
            print_instruction();
        }

        if (strncmp(buffer, ".memory", 7) == 0) {
            print_memory(buffer+8);
        }

        if (strncmp(buffer, ".goto", 5) == 0) {
            print_goto(buffer+6);
        }

        if (strncmp(buffer, ".step", 5) == 0) {
            print_instruction();

            if (step()) {
                print_registers();
                break;
            }

            print_registers();
        }

        if (strncmp(buffer, ".run", 4) == 0) {
            while (1) {
                if (step()) {
                    print_registers();
                    break;
                }
            }
        }
    }

cleanup:
    if (rom_data) {
        free(rom_data);
    }

    return rc;
}

void print_registers() {
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
}

void print_instruction() {
    int opcode_index = 0, mode = 0, arg0 = 0, arg1 = 0;
    char *mnemonic = NULL;

    opcode_index = (int)rom_data[registers.pc] & 0xFF;
    mode = opcodes[opcode_index].mode;
    mnemonic = opcodes[opcode_index].mnemonic;

    switch (mode) {
    case MODE_IMPLIED:
        printf("%s\n\n", mnemonic);
        break;
    case MODE_ACCUMULATOR:
        fprintf("%s A\n\n", mnemonic);
        break;
    case MODE_RELATIVE:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        if (arg0 >= 0x80) {
            printf("%s $%04X\n\n", mnemonic, registers.pc - (0xFF - arg0 - 1));
        } else {
            printf("%s $%04X\n\n", mnemonic, registers.pc + arg0);
        }
        break;
    case MODE_IMMEDIATE:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s #$%02X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE_X:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X, X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE_Y:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X, Y\n\n", mnemonic, arg0);
        break;
    case MODE_ABSOLUTE:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_ABSOLUTE_X:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X, X\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_ABSOLUTE_Y:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X, Y\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_INDIRECT:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (int)rom_data[registers.pc+2] & 0xFF;
        printf("%s ($%04X)\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_INDIRECT_X:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s ($%02X, X)\n\n", mnemonic, arg0);
        break;
    case MODE_INDIRECT_Y:
        arg0 = (int)rom_data[registers.pc+1] & 0xFF;
        printf("%s ($%02X), Y\n\n", mnemonic, arg0);
        break;
    default:
        break;
    }
}

int step() {
    int rc = RETURN_OK, opcode_index = 0, length = 0, arg0 = 0, arg1 = 0;

    opcode_index = (int)rom_data[registers.pc] & 0xFF;
    length = opcodes[opcode_index].length;

    if (!is_flag_undocumented() && (opcodes[opcode_index].meta & META_UNDOCUMENTED) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

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

cleanup:
    return rc;
}

void print_memory(char *input) {
    int a = 0, b = 0, i = 0, l = 0, counter = 0;
    size_t length = 0;

    length = strlen(input);

    if (length <= 5) {
        a = (int)strtol(input, NULL, 16);
        b = a + 1;
    } else if (length <= 10) {
        b = (int)strtol(input+5, NULL, 16);
        input[5] = '\0';
        a = (int)strtol(input, NULL, 16);

        b++;
    }

    printf("       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");

    for (i = a - (a % 0x10), l = b + (0x10 - (b % 0x10)); i < l; i++) {
        if ((counter % 16) == 0) {
            if (counter != 0) {
                printf("\n");
            }

            printf("%04X | ", i);
        } else {
            if ((counter % 8) == 0) {
                printf(" ");
            }
        }

        if (a <= i && i <= b) {
            printf("%02X ", (int)rom_data[i] & 0xFF);
        } else {
            printf("-- ");
        }

        counter++;
    }

    printf("\n");
}

void print_goto(char *input) {
    input[4] = '\0';

    registers.pc = (int)strtol(input, NULL, 16);
}

int do_aac(int opcode_index, int value) {

}

int do_aax(int opcode_index, int value) {

}

int do_adc(int opcode_index, int value) {
    int length = 0, mode = 0, data = 0, tmp = 0;

    length = opcodes[opcode_index].length;
    mode = opcodes[opcode_index].mode;

    if ((mode & MODE_IMMEDIATE) != 0) {
        data = value;
    } else {
        data = (int)rom_data[value];
    }

    tmp = registers.a + data + registers.flags.carry;
    registers.flags.overflow = ((!(((registers.a ^ data) & 0x80) != 0) && (((registers.a ^ tmp) & 0x80)) != 0) ? 1 : 0);
    registers.flags.carry = tmp > 255 ? 1 : 0;
    registers.flags.negative = (tmp >> 7) & 1;
    registers.flags.zero = (tmp & 0xFF) == 0 ? 1 : 0;
    registers.a = tmp & 255;
    registers.pc += length;
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
    int length = opcodes[opcode_index].length;

    rom_data[value] = (char)registers.a;

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
