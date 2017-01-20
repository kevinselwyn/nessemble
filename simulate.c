#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
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
unsigned int cycles = 0;

/**
 * Simulate usage
 */
void usage_simulate() {
    printf("Options:\n\n");
    printf("  .registers [register=XXXX,...]  Print registers (sets registers w/ options)\n");
    printf("  .flags [flag=X,...]             Print flags (sets flags w/ options)\n");
    printf("  .fill XXXX NN ...               Fill memory address with NN byte(s)\n");
    printf("  .instruction                    Print next instruction\n");
    printf("  .memory XXXX[:XXXX]             Print memory in address range\n");
    printf("  .goto XXXX                      Set program counter to XXXX\n");
    printf("  .step [X]                       Step program counter by 1 or X\n");
    printf("  .run                            Run program\n");
    printf("  .help                           Print this message\n\n");
}

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
        set_byte(i, 0xFF);
    }

    // load banks
    if (header == 1) {
        inesprg = (int)indata[4];

        if (inesprg == 1) {
            set_register(REGISTER_PC, 0xC000);
            memcpy(rom_data + get_register(REGISTER_PC), indata+0x10, 0x4000);
        } else {
            set_register(REGISTER_PC, 0x8000);
            memcpy(rom_data + get_register(REGISTER_PC), indata+0x10, 0x8000);
        }
    } else {
        set_register(REGISTER_PC, 0x8000);
        memcpy(rom_data + get_register(REGISTER_PC), indata, insize);
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

            if (repl(buffer+1) == TRUE) {
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

            if (repl(buffer+1) == TRUE) {
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

/**
 * Simulate REPL
 * @param {char *} input - User input
 * @return {int} Return code
 */
int repl(char *input) {
    int rc = RETURN_OK;
    size_t length = 0;

    length = strlen(input);

    if (strncmp(input, "help", 4) == 0) {
        usage_simulate();

        goto cleanup;
    }

    if (strncmp(input, "registers", 9) == 0) {
        if (length > 10) {
            load_registers(input+10);
            print_registers();
        } else {
            print_registers();
        }

        goto cleanup;
    }

    if (strncmp(input, "flags", 5) == 0) {
        if (length > 6) {
            load_flags(input+6);
            print_registers();
        } else {
            print_registers();
        }

        goto cleanup;
    }

    if (strncmp(input, "fill", 4) == 0) {
        if (length > 5) {
            print_memory(fill_memory(input+5));
        }

        goto cleanup;
    }

    if (strncmp(input, "instruction", 11) == 0) {
        print_instruction();

        goto cleanup;
    }

    if (strncmp(input, "memory", 6) == 0) {
        print_memory(input+7);

        goto cleanup;
    }

    if (strncmp(input, "cycles", 6) == 0) {
        print_cycles();

        goto cleanup;
    }

    if (strncmp(input, "goto", 4) == 0) {
        load_goto(input+5);

        goto cleanup;
    }

    if (strncmp(input, "step", 4) == 0) {
        if (length > 5) {
            steps(input+5);
        } else {
            steps("1");
        }

        goto cleanup;
    }

    if (strncmp(input, "run", 3) == 0) {
        while (1 == TRUE) {
            if (step() == TRUE) {
                print_registers();

                rc = RETURN_EPERM;
                goto cleanup;
            }
        }

        goto cleanup;
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

void load_flags(char *input) {
    unsigned int i = 0, l = 0, index = 0;
    unsigned int starts[10], ends[10];
    size_t length = 0;
    char text[13];

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
        if (strncmp(input+starts[i], "negative=", 9) == 0) {
            strncpy(text, input+starts[i]+9, (size_t)(ends[i] - starts[i] - 9));
            text[9] = '\0';
            registers.flags.negative = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "overflow=", 9) == 0) {
            strncpy(text, input+starts[i]+9, (size_t)(ends[i] - starts[i] - 9));
            text[9] = '\0';
            registers.flags.overflow = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "break=", 6) == 0) {
            strncpy(text, input+starts[i]+6, (size_t)(ends[i] - starts[i] - 6));
            text[6] = '\0';
            registers.flags.brk = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "decimal=", 8) == 0) {
            strncpy(text, input+starts[i]+8, (size_t)(ends[i] - starts[i] - 8));
            text[8] = '\0';
            registers.flags.decimal = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "interrupt=", 10) == 0) {
            strncpy(text, input+starts[i]+10, (size_t)(ends[i] - starts[i] - 10));
            text[10] = '\0';
            registers.flags.interrupt = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "zero=", 5) == 0) {
            strncpy(text, input+starts[i]+5, (size_t)(ends[i] - starts[i] - 5));
            text[5] = '\0';
            registers.flags.zero = (unsigned int)dec2int(text) & 1;
        } else if (strncmp(input+starts[i], "carry=", 6) == 0) {
            strncpy(text, input+starts[i]+6, (size_t)(ends[i] - starts[i] - 6));
            text[6] = '\0';
            registers.flags.carry = (unsigned int)dec2int(text) & 1;
        } else {
            continue;
        }
    }
}

char *fill_memory(char *input) {
    unsigned int i = 0, l = 0, addr_start = 0, addr_end = 0;
    size_t length = 0;
    char *addrs = NULL;

    addrs = malloc(sizeof(char) * 10);

    input[4] = '\0';
    addr_start = hex2int(input);
    addr_end = addr_start - 1;
    input[4] = ' ';

    length = strlen(input+5);

    for (i = 0, l = (int)(length / 3); i < l; i++) {
        rom_data[addr_start + i] = (char)hex2int(input + (5 + (i * 3)));
        addr_end++;
    }

    if (addr_end - addr_start == 0) {
        sprintf(addrs, "%04X", addr_start);
    } else {
        sprintf(addrs, "%04X:%04X", addr_start, addr_end);
    }

    return addrs;
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

int steps(char *input) {
    int rc = RETURN_OK;
    unsigned int i = 0, l = 0, count = 0;

    count = dec2int(input);

    for (i = 0, l = count; i < l; i++) {
        print_instruction();

        if (step() == TRUE) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        print_registers();
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
            printf("%02X ", get_byte(i) & 0xFF);
        } else {
            printf("-- ");
        }

        counter++;
    }

    printf("\n\n");
}

void print_cycles() {
    printf("%d cycles\n", cycles);
}

void load_goto(char *input) {
    input[4] = '\0';

    set_register(REGISTER_PC, hex2int(input));
}

unsigned int get_address(unsigned int opcode_index, unsigned int value) {
    int mode = opcodes[opcode_index].mode;
    unsigned int address = 0;

    switch (mode) {
    case MODE_ZEROPAGE:
    case MODE_ABSOLUTE:
        address = value;
        break;
    case MODE_ZEROPAGE_X:
    case MODE_ABSOLUTE_X:
        address = value + get_register(REGISTER_X);
        break;
    case MODE_ZEROPAGE_Y:
    case MODE_ABSOLUTE_Y:
        address = value + get_register(REGISTER_Y);
        break;
    case MODE_INDIRECT:
        address = ((get_byte(value + 0x01) & 0xFF) << 0x08) | (get_byte(value) & 0xFF);
        break;
    case MODE_INDIRECT_X:
        address = ((get_byte(value + get_register(REGISTER_X) + 0x01) & 0xFF) << 0x08) | (get_byte(value + get_register(REGISTER_X)) & 0xFF);
        break;
    case MODE_INDIRECT_Y:
        address = (((get_byte(value + 0x01) & 0xFF) << 0x08) | (get_byte(value) & 0xFF)) + get_register(REGISTER_Y);
        break;
    default:
        break;
    }

    return address;
}

unsigned int get_byte(unsigned int address) {
    return (unsigned int)rom_data[address] & 0xFF;
}

void set_byte(unsigned int address, unsigned int byte) {
    rom_data[address] = (char)byte;
}

unsigned int get_register(unsigned int reg) {
    unsigned int value = 0;

    switch (reg) {
    case REGISTER_A:
        value = registers.a;
        break;
    case REGISTER_X:
        value = registers.x;
        break;
    case REGISTER_Y:
        value = registers.y;
        break;
    case REGISTER_PC:
        value = (unsigned int)registers.pc;
        break;
    case REGISTER_SP:
        value = registers.sp;
        break;
    default:
        break;
    }

    return value;
}

void set_register(unsigned int reg, unsigned int value) {
    switch (reg) {
    case REGISTER_A:
        registers.a = value;
        break;
    case REGISTER_X:
        registers.x = value;
        break;
    case REGISTER_Y:
        registers.y = value;
        break;
    case REGISTER_PC:
        registers.pc = (int)value;
        break;
    case REGISTER_SP:
        registers.sp = value;
        break;
    default:
        break;
    }
}

void inc_register(unsigned int reg, int value) {
    switch (reg) {
    case REGISTER_A:
        registers.a += (unsigned int)value;
        break;
    case REGISTER_X:
        registers.x += (unsigned int)value;
        break;
    case REGISTER_Y:
        registers.y += (unsigned int)value;
        break;
    case REGISTER_PC:
        registers.pc += value;
        break;
    case REGISTER_SP:
        registers.sp += (unsigned int)value;
        break;
    default:
        break;
    }
}

void inc_cycles(unsigned int count) {
    cycles += count;
}

unsigned int get_flag(unsigned int flag) {
    unsigned int value = 0;

    switch (flag) {
    case FLG_NEGATIVE:
        value = registers.flags.negative;
        break;
    case FLG_OVERFLOW:
        value = registers.flags.overflow;
        break;
    case FLG_BREAK:
        value = registers.flags.brk;
        break;
    case FLG_DECIMAL:
        value = registers.flags.decimal;
        break;
    case FLG_INTERRUPT:
        value = registers.flags.interrupt;
        break;
    case FLG_ZERO:
        value = registers.flags.zero;
        break;
    case FLG_CARRY:
        value = registers.flags.carry;
        break;
    default:
        break;
    }

    return value;
}

void set_flag(unsigned int flag, unsigned int value) {
    switch (flag) {
    case FLG_NEGATIVE:
        registers.flags.negative = value;
        break;
    case FLG_OVERFLOW:
        registers.flags.overflow = value;
        break;
    case FLG_BREAK:
        registers.flags.brk = value;
        break;
    case FLG_DECIMAL:
        registers.flags.decimal = value;
        break;
    case FLG_INTERRUPT:
        registers.flags.interrupt = value;
        break;
    case FLG_ZERO:
        registers.flags.zero = value;
        break;
    case FLG_CARRY:
        registers.flags.carry = value;
        break;
    default:
        break;
    }
}

void stack_push(unsigned int value) {

}

unsigned int stack_pull() {
    inc_register(REGISTER_SP, 1);
    set_register(REGISTER_SP, get_register(REGISTER_SP) & 0xFF);

    return get_byte(0x0100 | get_register(REGISTER_SP));
}
