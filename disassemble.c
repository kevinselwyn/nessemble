#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

static unsigned int disassemble_offset(unsigned int offset, unsigned int inesprg, unsigned int ineschr, unsigned int inestrn) {
    if (inesprg == 0 && ineschr == 0) {
        return offset;
    }

    offset -= 0x10;

    if (inestrn == TRUE) {
        if (offset >= 0x200) {
            offset -= 0x200;
        } else {
            return offset;
        }
    }

    if (offset >= inesprg * BANK_PRG) {
        offset -= inesprg * BANK_PRG;
    } else {
        if (inesprg == 1) {
            offset += 0xC000;
        } else {
            offset = (offset + 0x8000) % 0x10000;

            if (offset < 0x8000) {
                offset += 0x8000;
            }
        }
    }

    return offset;
}

static unsigned int disassemble_byte(char *indata, unsigned int index) {
    return (unsigned int)indata[index] & 0xFF;
}

static void disassemble_inesprg(FILE *outfile, unsigned int offset, unsigned int inesprg) {
    fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; .inesprg %u\n", offset, inesprg, inesprg, inesprg);
}

static void disassemble_ineschr(FILE *outfile, unsigned int offset, unsigned int ineschr) {
    fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; .ineschr %u\n", offset, ineschr, ineschr, ineschr);
}

static void disassemble_inesmir(FILE *outfile, unsigned int offset, unsigned int arg0) {
    fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; .inesmir %u\n", offset, arg0, arg0, arg0 & 0x01);
}

static void disassemble_inesmap(FILE *outfile, unsigned int offset, unsigned int arg0, unsigned int arg1) {
    fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; .inesmap %u\n", offset, arg1, arg1, (arg0 >> 4) | (arg1 & 0xF0));
}

static void disassemble_db(FILE *outfile, unsigned int offset, unsigned int value) {
    fprintf(outfile, "%04X | %02X       | .db $%02X\n", offset, value, value);
}

static void disassemble_db_header(FILE *outfile, unsigned int offset, unsigned int value) {
    fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X\n", offset, value, value);
}

static void disassemble_implied(FILE *outfile, unsigned int offset, unsigned int opcode_id) {
    fprintf(outfile, "%04X | %02X       | %s\n", offset, opcode_id, opcodes[opcode_id].mnemonic);
}

static void disassemble_accumulator(FILE *outfile, unsigned int offset, unsigned int opcode_id) {
    fprintf(outfile, "%04X | %02X       | %s A\n", offset, opcode_id, opcodes[opcode_id].mnemonic);
}

static void disassemble_relative(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int value) {
    fprintf(outfile, "%04X | %02X %02X    | %s $%04X\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, value);
}

static void disassemble_immediate(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s #$%02X\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

static void disassemble_zeropage(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s <$%02X\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

static void disassemble_zeropage_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s <$%02X, X\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

static void disassemble_zeropage_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s <$%02X, Y\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

static void disassemble_absolute(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X\n", offset, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
}

static void disassemble_absolute_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X, X\n", offset, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
}

static void disassemble_absolute_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X, Y\n", offset, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
}

static void disassemble_indirect(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    fprintf(outfile, "%04X | %02X %02X %02X | %s [$%04X]\n", offset, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
}

static void disassemble_indirect_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s [$%02X, X]\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

static void disassemble_indirect_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    fprintf(outfile, "%04X | %02X %02X    | %s [$%02X], Y\n", offset, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
}

int disassemble(char *input, char *output) {
    int rc = RETURN_OK;
    unsigned int i = 0, l = 0, opcode_id = 0, arg0 = 0, arg1 = 0;
    unsigned int offset = 0, inesprg = 0, ineschr = 0, inestrn = FALSE, ines_header = FALSE;
    unsigned int insize = 0;
    char *indata = NULL;
    FILE *outfile = NULL;

    insize = load_file(&indata, input);

    if (!indata) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (insize == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    outfile = fopen(output, "w");

    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", output);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (strncmp(indata, "NES", 3) == 0) {
        fprintf(outfile, "0000 | XXXX | 4E 45 53 | .ascii \"NES\"\n");
        fprintf(outfile, "0003 | XXXX | 1A       | .db $1A\n");

        inesprg = disassemble_byte(indata, 4);
        disassemble_inesprg(outfile, 4, inesprg);

        ineschr = disassemble_byte(indata, 5);
        disassemble_ineschr(outfile, 5, ineschr);

        arg0 = disassemble_byte(indata, 6);
        inestrn = (unsigned int)((arg0 & 0x04) != 0 ? TRUE : FALSE);
        disassemble_inesmir(outfile, 6, arg0);
        disassemble_inesmap(outfile, 7, arg0, disassemble_byte(indata, 7));

        disassemble_db_header(outfile, 8, disassemble_byte(indata, 8));
        disassemble_db_header(outfile, 9, disassemble_byte(indata, 9));
        disassemble_db_header(outfile, 10, disassemble_byte(indata, 10));

        for (i = 11, l = 16; i < l; i++) {
            disassemble_db_header(outfile, i, disassemble_byte(indata, i));
        }

        i = 0x10;
    }

    ines_header = (unsigned int)(inesprg != 0 || ineschr != 0 ? TRUE : FALSE);

    for (l = (unsigned int)insize; i < l; i++) {
        opcode_id = disassemble_byte(indata, i);
        offset = disassemble_offset(i, inesprg, ineschr, inestrn);

        if (ines_header == TRUE) {
            fprintf(outfile, "%04X | ", i);
        }

        if ((opcodes[opcode_id].meta & META_UNDOCUMENTED) == 0) {
            if (i + (opcodes[opcode_id].length - 1) >= insize) {
                disassemble_db(outfile, offset, opcode_id);

                i += 1;
                continue;
            }

            arg0 = i + 1 < l ? disassemble_byte(indata, i + 1) : 0;
            arg1 = i + 2 < l ? disassemble_byte(indata, i + 2) : 0;

            switch (opcodes[opcode_id].mode) {
            case MODE_IMPLIED:
                disassemble_implied(outfile, offset, opcode_id);
                break;
            case MODE_ACCUMULATOR:
                disassemble_accumulator(outfile, offset, opcode_id);
                break;
            case MODE_RELATIVE:
                if (arg0 >= 0x80) {
                    disassemble_relative(outfile, offset, opcode_id, arg0, offset - (0xFF - arg0 - 1));
                } else {
                    disassemble_relative(outfile, offset, opcode_id, arg0, offset + arg0 + 2);
                }

                i++;
                break;
            case MODE_IMMEDIATE:
                disassemble_immediate(outfile, offset, opcode_id, arg0);

                i++;
                break;
            case MODE_ZEROPAGE:
                disassemble_zeropage(outfile, offset, opcode_id, arg0);

                i++;
                break;
            case MODE_ZEROPAGE_X:
                disassemble_zeropage_x(outfile, offset, opcode_id, arg0);

                i++;
                break;
            case MODE_ZEROPAGE_Y:
                disassemble_zeropage_y(outfile, offset, opcode_id, arg0);

                i++;
                break;
            case MODE_ABSOLUTE:
                disassemble_absolute(outfile, offset, opcode_id, arg0, arg1);

                i += 2;
                break;
            case MODE_ABSOLUTE_X:
                disassemble_absolute_x(outfile, offset, opcode_id, arg0, arg1);

                i += 2;
                break;
            case MODE_ABSOLUTE_Y:
                disassemble_absolute_y(outfile, offset, opcode_id, arg0, arg1);

                i += 2;
                break;
            case MODE_INDIRECT:
                disassemble_indirect(outfile, offset, opcode_id, arg0, arg1);

                i += 2;
                break;
            case MODE_INDIRECT_X:
                disassemble_indirect_x(outfile, offset, opcode_id, arg0);

                i++;
                break;
            case MODE_INDIRECT_Y:
                disassemble_indirect_y(outfile, offset, opcode_id, arg0);

                i++;
                break;
            default:
                disassemble_db(outfile, offset, opcode_id);

                break;
            }
        } else {
            disassemble_db(outfile, offset, opcode_id);
        }
    }

cleanup:
    nessemble_free(indata);

    if (outfile) {
        (void)fclose(outfile);
    }

    return rc;
}
