#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

// https://github.com/bfirsh/jsnes/blob/master/source/cpu.js

struct flgs {
    unsigned int negative, overflow, brk, decimal, ineterrupt, zero, carry;
};

struct regs {
    unsigned int a, x, y;
    int pc;
    unsigned int sp;
    struct flgs flags;
};

struct regs registers = { 0, 0, 0, 0x8000, 0xFF, { 0, 0, 0, 0, 0, 0, 0 } };

#define BUF_SIZE 256

char *rom_data = NULL;
char buffer[BUF_SIZE];

/**
 * Simulate 6502
 * @param {char *} input - Input filename
 */
int simulate(char *input, char *recipe) {
    int rc = RETURN_OK, i = 0, l = 0, header = 0;
    int inesprg = 1, ineschr = 1;
    size_t insize = 0;
    char *indata = NULL;
    FILE *recipe_file = NULL;

    insize = load_file(&indata, input);

    if (insize == 0) {
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
    if (recipe) {
        recipe_file = fopen(recipe, "r");

        if (!recipe_file) {
            fprintf(stderr, "Could not open %s\n", recipe);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        while (fgets(buffer, BUF_SIZE, recipe_file) != NULL) {
            if (registers.pc == -1) {
                break;
            }

            if (repl(buffer)) {
                break;
            }
        }
    } else {
        while (1) {
            if (registers.pc == -1) {
                break;
            }

            printf("nessemble> ");

            fgets(buffer, BUF_SIZE, stdin);

            if (repl(buffer)) {
                break;
            }
        }
    }

cleanup:
    if (rom_data) {
        free(rom_data);
    }

    if (recipe_file) {
        fclose(recipe_file);
    }

    return rc;
}

int repl(char *input) {
    int rc = RETURN_OK;
    size_t length = 0;

    length = strlen(input);

    if (strncmp(input, ".registers", 10) == 0) {
        if (length > 11) {
            load_registers(buffer+11);
            print_registers();
        } else {
            print_registers();
        }
    }

    if (strncmp(input, ".instruction", 12) == 0) {
        print_instruction();
    }

    if (strncmp(input, ".memory", 7) == 0) {
        print_memory(input+8);
    }

    if (strncmp(input, ".goto", 5) == 0) {
        load_goto(input+6);
    }

    if (strncmp(input, ".step", 5) == 0) {
        print_instruction();

        if (step()) {
            print_registers();

            rc = RETURN_EPERM;
            goto cleanup;
        }

        print_registers();
    }

    if (strncmp(input, ".run", 4) == 0) {
        while (1) {
            if (step()) {
                print_registers();

                rc = RETURN_EPERM;
                goto cleanup;
            }
        }
    }

cleanup:
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

void load_registers(char *input) {
    int i = 0, l = 0, index = 0;
    int starts[10], ends[10];
    size_t length = 0;
    char text[8];

    length = strlen(input);

    starts[index] = 0;

    for (i = 0, l = (int)length + 1; i < l; i++) {
        if (input[i] == ',') {
            ends[index++] = i;
            starts[index] = i + 1;
        }

        if (input[i] == '\0') {
            ends[index++] = i - 1;
        }
    }

    for (i = 0, l = index; i < l; i++) {
        if (strncmp(input+starts[i], "A=", 2) == 0) {
            strncpy(text, input+starts[i]+2, (size_t)(ends[i] - starts[i] - 2));
            text[2] = '\0';
            registers.a = (int)strtol(text, NULL, 16) & 0xFF;
        } else if (strncmp(input+starts[i], "X=", 2) == 0) {
            strncpy(text, input+starts[i]+2, (size_t)(ends[i] - starts[i] - 2));
            text[2] = '\0';
            registers.x = (int)strtol(text, NULL, 16) & 0xFF;
        } else if (strncmp(input+starts[i], "Y=", 2) == 0) {
            strncpy(text, input+starts[i]+2, (size_t)(ends[i] - starts[i] - 2));
            text[2] = '\0';
            registers.y = (int)strtol(text, NULL, 16) & 0xFF;
        } else if (strncmp(input+starts[i], "PC=", 3) == 0) {
            strncpy(text, input+starts[i]+3, (size_t)(ends[i] - starts[i] - 3));
            text[4] = '\0';
            registers.pc = (int)strtol(text, NULL, 16) & 0xFFFF;
        } else if (strncmp(input+starts[i], "SP=", 3) == 0) {
            strncpy(text, input+starts[i]+3, (size_t)(ends[i] - starts[i] - 3));
            text[4] = '\0';
            registers.sp = (int)strtol(text, NULL, 16) & 0xFFFF;
        } else {
            continue;
        }
    }
}

void print_instruction() {
    int opcode_index = 0, mode = 0;
    unsigned int arg0 = 0, arg1 = 0;
    char *mnemonic = NULL;

    opcode_index = (int)rom_data[registers.pc] & 0xFF;
    mode = opcodes[opcode_index].mode;
    mnemonic = opcodes[opcode_index].mnemonic;

    switch (mode) {
    case MODE_IMPLIED:
        printf("%s\n\n", mnemonic);
        break;
    case MODE_ACCUMULATOR:
        printf("%s A\n\n", mnemonic);
        break;
    case MODE_RELATIVE:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        if (arg0 >= 0x80) {
            printf("%s $%04X\n\n", mnemonic, registers.pc - (0xFF - arg0 - 1));
        } else {
            printf("%s $%04X\n\n", mnemonic, registers.pc + arg0);
        }
        break;
    case MODE_IMMEDIATE:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        printf("%s #$%02X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE_X:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X, X\n\n", mnemonic, arg0);
        break;
    case MODE_ZEROPAGE_Y:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        printf("%s <$%02X, Y\n\n", mnemonic, arg0);
        break;
    case MODE_ABSOLUTE:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (unsigned int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_ABSOLUTE_X:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (unsigned int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X, X\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_ABSOLUTE_Y:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (unsigned int)rom_data[registers.pc+2] & 0xFF;
        printf("%s $%04X, Y\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_INDIRECT:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        arg1 = (unsigned int)rom_data[registers.pc+2] & 0xFF;
        printf("%s ($%04X)\n\n", mnemonic, (arg1 << 8) | arg0);
        break;
    case MODE_INDIRECT_X:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
        printf("%s ($%02X, X)\n\n", mnemonic, arg0);
        break;
    case MODE_INDIRECT_Y:
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;
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

    if (is_flag_undocumented() == FALSE && (opcodes[opcode_index].meta & META_UNDOCUMENTED) != 0) {
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
        b = a;
    } else if (length <= 10) {
        b = (int)strtol(input+5, NULL, 16);
        input[5] = '\0';
        a = (int)strtol(input, NULL, 16);

        b++;
    }

    printf("       00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F\n");

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
            printf("%02X ", (unsigned int)rom_data[i] & 0xFF);
        } else {
            printf("-- ");
        }

        counter++;
    }

    printf("\n\n");
}

void load_goto(char *input) {
    input[4] = '\0';

    registers.pc = (int)strtol(input, NULL, 16);
}

unsigned int get_data(int mode, int value) {
    unsigned int data = 0;

    if ((mode & MODE_IMMEDIATE) != 0) {
        data = value & 0xFF;
    } else if ((mode & MODE_ZEROPAGE_X) != 0 || (mode & MODE_ABSOLUTE_X) != 0) {
        data = (unsigned int)rom_data[value + registers.x] & 0xFF;
    } else if ((mode & MODE_ZEROPAGE_Y) != 0 || (mode & MODE_ABSOLUTE_Y) != 0) {
        data = (unsigned int)rom_data[value + registers.y] & 0xFF;
    } else {
        data = (unsigned int)rom_data[value] & 0xFF;
    }

    return data;
}

void do_aac(int opcode_index, int value) {

}

void do_aax(int opcode_index, int value) {

}

void do_adc(int opcode_index, int value) {
    unsigned int length = 0, mode = 0, data = 0, tmp = 0;

    length = (unsigned int)opcodes[opcode_index].length;
    mode = (unsigned int)opcodes[opcode_index].mode;

    data = get_data(mode, value);

    tmp = registers.a + data + registers.flags.carry;
    registers.flags.overflow = ((!(((registers.a ^ data) & 0x80) != 0) && (((registers.a ^ tmp) & 0x80)) != 0) ? 1 : 0);
    registers.flags.carry = tmp > 0xFF ? 1 : 0;
    registers.flags.negative = (tmp >> 0x07) & 0x01;
    registers.flags.zero = (tmp & 0xFF) == 0 ? 1 : 0;
    registers.a = tmp & 0xFF;
    registers.pc += (int)length;
}

void do_and(int opcode_index, int value) {
    unsigned int length = 0, mode = 0, data = 0;

    length = (unsigned int)opcodes[opcode_index].length;
    mode = (unsigned int)opcodes[opcode_index].mode;

    data = get_data(mode, value);

    registers.a = registers.a & value;
    registers.flags.negative = (registers.a >> 0x07) & 1;
    registers.flags.zero = registers.a == 0 ? 1 : 0;
    registers.pc += (int)length;
}

void do_arr(int opcode_index, int value) {

}

void do_asl(int opcode_index, int value) {
    unsigned int length = 0, mode = 0, tmp = 0;

    length = (unsigned int)opcodes[opcode_index].length;
    mode = (unsigned int)opcodes[opcode_index].mode;

    if ((mode & MODE_ACCUMULATOR) != 0) {
        registers.flags.carry = (registers.a >> 0x07) & 0x01;
        registers.a = (registers.a << 0x01) & 0xFF;
        registers.flags.negative = (registers.a >> 0x07) & 0x01;
        registers.flags.zero = (registers.a & 0xFF) == 0 ? 1 : 0;
    } else {
        tmp = (unsigned int)rom_data[value] & 0xFF;
        registers.flags.carry = (tmp >> 0x07) & 0x01;
        tmp = (tmp << 0x01) & 0xFF;
        registers.flags.negative = (tmp >> 0x07) & 0x01;
        registers.flags.zero = (tmp & 0xFF) == 0 ? 1 : 0;
        rom_data[value] = (char)tmp;
    }

    registers.pc += (int)length;
}

void do_asr(int opcode_index, int value) {

}

void do_atx(int opcode_index, int value) {

}

void do_axa(int opcode_index, int value) {

}

void do_axs(int opcode_index, int value) {

}

void do_bcc(int opcode_index, int value) {

}

void do_bcs(int opcode_index, int value) {

}

void do_beq(int opcode_index, int value) {

}

void do_bit(int opcode_index, int value) {

}

void do_bmi(int opcode_index, int value) {

}

void do_bne(int opcode_index, int value) {

}

void do_bpl(int opcode_index, int value) {

}

void do_brk(int opcode_index, int value) {

}

void do_bvc(int opcode_index, int value) {

}

void do_bvs(int opcode_index, int value) {

}

void do_clc(int opcode_index, int value) {

}

void do_cld(int opcode_index, int value) {

}

void do_cli(int opcode_index, int value) {

}

void do_clv(int opcode_index, int value) {

}

void do_cmp(int opcode_index, int value) {

}

void do_cpx(int opcode_index, int value) {

}

void do_cpy(int opcode_index, int value) {

}

void do_dcp(int opcode_index, int value) {

}

void do_dec(int opcode_index, int value) {

}

void do_dex(int opcode_index, int value) {

}

void do_dey(int opcode_index, int value) {

}

void do_dop(int opcode_index, int value) {

}

void do_eor(int opcode_index, int value) {

}

void do_inc(int opcode_index, int value) {

}

void do_inx(int opcode_index, int value) {

}

void do_iny(int opcode_index, int value) {

}

void do_isc(int opcode_index, int value) {

}

void do_jmp(int opcode_index, int value) {

}

void do_jsr(int opcode_index, int value) {

}

void do_kil(int opcode_index, int value) {

}

void do_lar(int opcode_index, int value) {

}

void do_lax(int opcode_index, int value) {

}

void do_lda(int opcode_index, int value) {
    unsigned int length = 0, mode = 0;

    length = (unsigned int)opcodes[opcode_index].length;
    mode = (unsigned int)opcodes[opcode_index].mode;

    if ((mode & MODE_IMMEDIATE) != 0) {
        registers.a = (unsigned int)value & 0xFF;
    } else {
        registers.a = (unsigned int)rom_data[value] & 0xFF;
    }

    registers.flags.negative = (registers.a >> 7) & 1;
    registers.flags.zero = registers.a == 0 ? 1 : 0;
    registers.pc += (int)length;
}

void do_ldx(int opcode_index, int value) {

}

void do_ldy(int opcode_index, int value) {

}

void do_lsr(int opcode_index, int value) {

}

void do_nop(int opcode_index, int value) {

}

void do_ora(int opcode_index, int value) {

}

void do_pha(int opcode_index, int value) {

}

void do_php(int opcode_index, int value) {

}

void do_pla(int opcode_index, int value) {

}

void do_plp(int opcode_index, int value) {

}

void do_rla(int opcode_index, int value) {

}

void do_rol(int opcode_index, int value) {

}

void do_ror(int opcode_index, int value) {

}

void do_rra(int opcode_index, int value) {

}

void do_rti(int opcode_index, int value) {

}

void do_rts(int opcode_index, int value) {

}

void do_sbc(int opcode_index, int value) {

}

void do_sec(int opcode_index, int value) {

}

void do_sed(int opcode_index, int value) {

}

void do_sei(int opcode_index, int value) {

}

void do_slo(int opcode_index, int value) {

}

void do_sre(int opcode_index, int value) {

}

void do_sta(int opcode_index, int value) {
    unsigned int length = (unsigned int)opcodes[opcode_index].length;

    rom_data[value] = (char)registers.a;

    registers.pc += (int)length;
}

void do_stx(int opcode_index, int value) {

}

void do_sty(int opcode_index, int value) {

}

void do_sxa(int opcode_index, int value) {

}

void do_sya(int opcode_index, int value) {

}

void do_tax(int opcode_index, int value) {

}

void do_tay(int opcode_index, int value) {

}

void do_top(int opcode_index, int value) {

}

void do_tsx(int opcode_index, int value) {

}

void do_txa(int opcode_index, int value) {

}

void do_txs(int opcode_index, int value) {

}

void do_tya(int opcode_index, int value) {

}

void do_xaa(int opcode_index, int value) {

}

void do_xas(int opcode_index, int value) {

}
