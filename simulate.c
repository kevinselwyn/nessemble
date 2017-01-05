#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

// https://github.com/bfirsh/jsnes/blob/master/source/cpu.js

#define BUF_SIZE 256

struct flgs {
    unsigned int negative, overflow, brk, decimal, interrupt, zero, carry;
};

struct regs {
    unsigned int a, x, y;
    int pc;
    unsigned int sp;
    struct flgs flags;
};

static struct regs registers = { 0, 0, 0, 0x8000, 0xFF, { 0, 0, 0, 0, 0, 0, 0 } };

char *rom_data;

/**
 * Simulate 6502
 * @param {char *} input - Input filename
 */
int simulate(char *input, char *recipe) {
    int rc = RETURN_OK, i = 0, l = 0, header = 0;
    int inesprg = 1;
    size_t insize = 0;
    char buffer[BUF_SIZE];
    char *indata = NULL;
    FILE *recipe_file = NULL;

    insize = load_file(&indata, input);

    if (!indata) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

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
        rom_data[i] = (char)0xFF;
    }

    // load banks
    if (header == 1) {
        inesprg = (int)indata[4];

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
    if (recipe != NULL && strcmp(recipe, "") != 0) {
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

            if (repl(buffer) == TRUE) {
                break;
            }
        }
    } else {
        while (1 == TRUE) {
            if (registers.pc == -1) {
                break;
            }

            printf("nessemble> ");

            if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
                break;
            }

            if (repl(buffer) == TRUE) {
                break;
            }
        }
    }

cleanup:
    if (rom_data) {
        free(rom_data);
    }

    if (recipe_file) {
        (void)fclose(recipe_file);
    }

    return rc;
}

int repl(char *input) {
    int rc = RETURN_OK;
    size_t length = 0;

    length = strlen(input);

    if (strncmp(input, ".registers", 10) == 0) {
        if (length > 11) {
            load_registers(input+11);
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

        if (step() == TRUE) {
            print_registers();

            rc = RETURN_EPERM;
            goto cleanup;
        }

        print_registers();
    }

    if (strncmp(input, ".run", 4) == 0) {
        while (1 == TRUE) {
            if (step() == TRUE) {
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
    printf("%04X ", (unsigned int)registers.pc);
    printf("%02X ", registers.a);
    printf("%02X ", registers.x);
    printf("%02X ", registers.y);
    printf("%02X ", registers.sp);

    printf("%u", registers.flags.negative & 1);
    printf("%u0", registers.flags.overflow & 1);
    printf("%u", registers.flags.brk & 1);
    printf("%u", registers.flags.decimal & 1);
    printf("%u", registers.flags.interrupt & 1);
    printf("%u", registers.flags.zero & 1);
    printf("%u", registers.flags.carry & 1);

    printf("\n\n");
}

void load_registers(char *input) {
    unsigned int i = 0, l = 0, index = 0;
    unsigned int starts[10], ends[10];
    size_t length = 0;
    char text[8];

    length = strlen(input);

    for (i = 0, l = 10; i < l; i++) {
        starts[i] = 0;
        ends[i] = 0;
    }

    for (i = 0, l = (unsigned int)length + 1; i < l; i++) {
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
            registers.a = (unsigned int)hex2int(text) & 0xFF;
        } else if (strncmp(input+starts[i], "X=", 2) == 0) {
            strncpy(text, input+starts[i]+2, (size_t)(ends[i] - starts[i] - 2));
            text[2] = '\0';
            registers.x = (unsigned int)hex2int(text) & 0xFF;
        } else if (strncmp(input+starts[i], "Y=", 2) == 0) {
            strncpy(text, input+starts[i]+2, (size_t)(ends[i] - starts[i] - 2));
            text[2] = '\0';
            registers.y = (unsigned int)hex2int(text) & 0xFF;
        } else if (strncmp(input+starts[i], "PC=", 3) == 0) {
            strncpy(text, input+starts[i]+3, (size_t)(ends[i] - starts[i] - 3));
            text[4] = '\0';
            registers.pc = (int)hex2int(text) & 0xFFFF;
        } else if (strncmp(input+starts[i], "SP=", 3) == 0) {
            strncpy(text, input+starts[i]+3, (size_t)(ends[i] - starts[i] - 3));
            text[4] = '\0';
            registers.sp = (unsigned int)hex2int(text) & 0xFFFF;
        } else {
            continue;
        }
    }
}

void print_instruction() {
    unsigned int opcode_index = 0, mode = 0, arg0 = 0, arg1 = 0;
    char *mnemonic = NULL;

    opcode_index = (unsigned int)rom_data[registers.pc] & 0xFF;
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
            printf("%s $%04X\n\n", mnemonic, (unsigned int)(registers.pc - (0xFF - arg0 - 1)));
        } else {
            printf("%s $%04X\n\n", mnemonic, (unsigned int)(registers.pc + arg0));
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
    int rc = RETURN_OK;
    unsigned int opcode_index = 0, length = 0, arg0 = 0, arg1 = 0;

    opcode_index = (unsigned int)rom_data[registers.pc] & 0xFF;
    length = opcodes[opcode_index].length;

    if (is_flag_undocumented() == FALSE && (opcodes[opcode_index].meta & META_UNDOCUMENTED) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (length == 1) {
        (*opcodes[opcode_index].func)(opcode_index, 0);
    } else {
        arg0 = (unsigned int)rom_data[registers.pc+1] & 0xFF;

        if (length == 2) {
            (*opcodes[opcode_index].func)(opcode_index, arg0);
        } else if (length == 3) {
            arg1 = (unsigned int)rom_data[registers.pc+2] & 0xFF;
            (*opcodes[opcode_index].func)(opcode_index, (arg1 << 8) | arg0);
        }
    }

cleanup:
    return rc;
}

void print_memory(char *input) {
    unsigned int a = 0, b = 0, i = 0, l = 0, counter = 0;
    size_t length = 0;

    length = strlen(input);

    if (length <= 5) {
        a = (unsigned int)hex2int(input);
        b = a;
    } else if (length <= 10) {
        b = (unsigned int)hex2int(input+5);
        input[5] = '\0';
        a = (unsigned int)hex2int(input);

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

    registers.pc = hex2int(input);
}

unsigned int get_address(unsigned int opcode_index, unsigned int value) {
    int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;

    if (mode == MODE_ZEROPAGE || mode == MODE_ABSOLUTE) {
        address = value;
    } else if (mode == MODE_ZEROPAGE_X || mode == MODE_ABSOLUTE_X) {
        address = value + registers.x;
    } else if (mode == MODE_ZEROPAGE_Y || mode == MODE_ABSOLUTE_Y) {
        address = value + registers.y;
    }

    return address;
}

void do_aac(unsigned int opcode_index, unsigned int value) {

}

void do_aax(unsigned int opcode_index, unsigned int value) {

}

void do_adc(unsigned int opcode_index, unsigned int value) {

}

void do_and(unsigned int opcode_index, unsigned int value) {

}

void do_arr(unsigned int opcode_index, unsigned int value) {

}

void do_asl(unsigned int opcode_index, unsigned int value) {

}

void do_asr(unsigned int opcode_index, unsigned int value) {

}

void do_atx(unsigned int opcode_index, unsigned int value) {

}

void do_axa(unsigned int opcode_index, unsigned int value) {

}

void do_axs(unsigned int opcode_index, unsigned int value) {

}

void do_bcc(unsigned int opcode_index, unsigned int value) {

}

void do_bcs(unsigned int opcode_index, unsigned int value) {

}

void do_beq(unsigned int opcode_index, unsigned int value) {

}

void do_bit(unsigned int opcode_index, unsigned int value) {

}

void do_bmi(unsigned int opcode_index, unsigned int value) {

}

void do_bne(unsigned int opcode_index, unsigned int value) {

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

}

void do_cld(unsigned int opcode_index, unsigned int value) {

}

void do_cli(unsigned int opcode_index, unsigned int value) {

}

void do_clv(unsigned int opcode_index, unsigned int value) {

}

void do_cmp(unsigned int opcode_index, unsigned int value) {

}

void do_cpx(unsigned int opcode_index, unsigned int value) {

}

void do_cpy(unsigned int opcode_index, unsigned int value) {

}

void do_dcp(unsigned int opcode_index, unsigned int value) {

}

void do_dec(unsigned int opcode_index, unsigned int value) {

}

void do_dex(unsigned int opcode_index, unsigned int value) {

}

void do_dey(unsigned int opcode_index, unsigned int value) {

}

void do_dop(unsigned int opcode_index, unsigned int value) {

}

void do_eor(unsigned int opcode_index, unsigned int value) {

}

void do_inc(unsigned int opcode_index, unsigned int value) {

}

void do_inx(unsigned int opcode_index, unsigned int value) {

}

void do_iny(unsigned int opcode_index, unsigned int value) {

}

void do_isc(unsigned int opcode_index, unsigned int value) {

}

void do_jmp(unsigned int opcode_index, unsigned int value) {

}

void do_jsr(unsigned int opcode_index, unsigned int value) {

}

void do_kil(unsigned int opcode_index, unsigned int value) {

}

void do_lar(unsigned int opcode_index, unsigned int value) {

}

void do_lax(unsigned int opcode_index, unsigned int value) {

}

void do_lda(unsigned int opcode_index, unsigned int value) {

}

void do_ldx(unsigned int opcode_index, unsigned int value) {

}

void do_ldy(unsigned int opcode_index, unsigned int value) {

}

void do_lsr(unsigned int opcode_index, unsigned int value) {

}

void do_nop(unsigned int opcode_index, unsigned int value) {

}

void do_ora(unsigned int opcode_index, unsigned int value) {

}

void do_pha(unsigned int opcode_index, unsigned int value) {

}

void do_php(unsigned int opcode_index, unsigned int value) {

}

void do_pla(unsigned int opcode_index, unsigned int value) {

}

void do_plp(unsigned int opcode_index, unsigned int value) {

}

void do_rla(unsigned int opcode_index, unsigned int value) {

}

void do_rol(unsigned int opcode_index, unsigned int value) {

}

void do_ror(unsigned int opcode_index, unsigned int value) {

}

void do_rra(unsigned int opcode_index, unsigned int value) {

}

void do_rti(unsigned int opcode_index, unsigned int value) {

}

void do_rts(unsigned int opcode_index, unsigned int value) {

}

void do_sbc(unsigned int opcode_index, unsigned int value) {

}

void do_sec(unsigned int opcode_index, unsigned int value) {
    registers.flags.carry = 1;
    registers.pc += opcodes[opcode_index].length;
}

void do_sed(unsigned int opcode_index, unsigned int value) {
    registers.flags.decimal = 1;
    registers.pc += opcodes[opcode_index].length;
}

void do_sei(unsigned int opcode_index, unsigned int value) {
    registers.flags.interrupt = 1;
    registers.pc += opcodes[opcode_index].length;
}

void do_slo(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sre(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sta(unsigned int opcode_index, unsigned int value) {

}

void do_stx(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    rom_data[address] = (char)registers.x;
    registers.pc += opcodes[opcode_index].length;
}

void do_sty(unsigned int opcode_index, unsigned int value) {
    unsigned int address = get_address(opcode_index, value);

    rom_data[address] = (char)registers.y;
    registers.pc += opcodes[opcode_index].length;
}

void do_sxa(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_sya(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_tax(unsigned int opcode_index, unsigned int value) {
    registers.x = registers.a;
    registers.flags.negative = (registers.a >> 0x07) & 1;
    registers.flags.zero = (unsigned int)(registers.a == 0 ? 1 : 0);
    registers.pc += opcodes[opcode_index].length;
}

void do_tay(unsigned int opcode_index, unsigned int value) {
    registers.y = registers.a;
    registers.flags.negative = (registers.a >> 0x07) & 1;
    registers.flags.zero = (unsigned int)(registers.a == 0 ? 1 : 0);
    registers.pc += opcodes[opcode_index].length;
}

void do_top(unsigned int opcode_index, unsigned int value) {
    // TODO: Undocumented
}

void do_tsx(unsigned int opcode_index, unsigned int value) {
    registers.x = registers.sp;
    registers.pc += opcodes[opcode_index].length;
}

void do_txa(unsigned int opcode_index, unsigned int value) {
    registers.a = registers.x;
    registers.flags.negative = (registers.x >> 0x07) & 1;
    registers.flags.zero = (unsigned int)(registers.x == 0 ? 1 : 0);
    registers.pc += opcodes[opcode_index].length;
}

void do_txs(unsigned int opcode_index, unsigned int value) {
    registers.sp = registers.x;
    registers.pc += opcodes[opcode_index].length;
}

void do_tya(unsigned int opcode_index, unsigned int value) {
    registers.a = registers.y;
    registers.flags.negative = (registers.y >> 0x07) & 1;
    registers.flags.zero = (unsigned int)(registers.y == 0 ? 1 : 0);
    registers.pc += opcodes[opcode_index].length;
}

void do_xaa(unsigned int opcode_index, unsigned int value) {

}

void do_xas(unsigned int opcode_index, unsigned int value) {

}
