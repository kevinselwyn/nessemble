#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

unsigned int reassemblable = FALSE;

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
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | ", offset, inesprg);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X ; .inesprg %u\n", inesprg, inesprg);
}

static void disassemble_ineschr(FILE *outfile, unsigned int offset, unsigned int ineschr) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | ", offset, ineschr);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X ; .ineschr %u\n", ineschr, ineschr);
}

static void disassemble_inesmir(FILE *outfile, unsigned int offset, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | ", offset, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X ; .inesmir %u\n", arg0, arg0 & 0x01);
}

static void disassemble_inesmap(FILE *outfile, unsigned int offset, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | ", offset, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X ; .inesmap %u\n", arg1, (arg0 >> 4) | (arg1 & 0xF0));
}

static void disassemble_db_header(FILE *outfile, unsigned int offset, unsigned int value) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | ", offset, value);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X\n", value);
}

static void disassemble_db(FILE *outfile, unsigned int offset, unsigned int value) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X       | ", offset, value);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".db $%02X", value);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_implied(FILE *outfile, unsigned int offset, unsigned int opcode_id) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X       | ", offset, opcode_id);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s", opcodes[opcode_id].mnemonic);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_accumulator(FILE *outfile, unsigned int offset, unsigned int opcode_id) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X       | ", offset, opcode_id);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s A", opcodes[opcode_id].mnemonic);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_relative(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int value) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s $%04X", opcodes[opcode_id].mnemonic, value);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_immediate(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s #$%02X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_zeropage(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s <$%02X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_zeropage_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s <$%02X, X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_zeropage_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s <$%02X, Y", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_absolute(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s $%04X", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_absolute_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s $%04X, X", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_absolute_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s $%04X, Y", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_indirect(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s [$%04X]", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_indirect_x(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s [$%02X, X]", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

static void disassemble_indirect_y(FILE *outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, "%s [$%02X], Y", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        fprintf(outfile, " ; %04X", offset);
    }

    fprintf(outfile, "\n");
}

int disassemble(char *input, char *output, char *listname) {
    int rc = RETURN_OK;
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int opcode_id = 0, arg0 = 0, arg1 = 0;
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

    reassemblable = is_flag_reassemble();

    if (strncmp(indata, "NES", 3) == 0) {
        if (reassemblable == FALSE) {
            fprintf(outfile, "0000 | XXXX | 4E 45 53 | ");
        }

        fprintf(outfile, ".ascii \"NES\"");

        if (reassemblable  == TRUE) {
            fprintf(outfile, " ; header");
        }

        fprintf(outfile, "\n");

        if (reassemblable == FALSE) {
            fprintf(outfile, "0003 | XXXX | 1A       | ");
        }

        fprintf(outfile, ".db $1A\n");

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

        for (i = 11, j = 16; i < j; i++) {
            disassemble_db_header(outfile, i, disassemble_byte(indata, i));
        }

        i = 0x10;
    }

    ines_header = (unsigned int)(inesprg != 0 || ineschr != 0 ? TRUE : FALSE);

    if (ines_header == TRUE && reassemblable == TRUE) {
        fprintf(outfile, "\n");
    }

    if (input_list(listname) != RETURN_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (j = (unsigned int)insize; i < j; i++) {
        opcode_id = disassemble_byte(indata, i);
        offset = disassemble_offset(i, inesprg, ineschr, inestrn);

        if (ines_header == TRUE && reassemblable == FALSE) {
            fprintf(outfile, "%04X | ", i);
        }

        if (reassemblable == TRUE) {
            for (k = 0, l = symbol_index; k < l; k++) {
                if (symbols[k].type == SYMBOL_LABEL && symbols[k].value == offset) {
                    fprintf(outfile, "%s: ; %04X\n", symbols[k].name, symbols[k].value);
                }
            }
        }

        if ((opcodes[opcode_id].meta & META_UNDOCUMENTED) == 0) {
            if (i + (opcodes[opcode_id].length - 1) >= insize) {
                disassemble_db(outfile, offset, opcode_id);

                i += 1;
                continue;
            }

            arg0 = i + 1 < j ? disassemble_byte(indata, i + 1) : 0;
            arg1 = i + 2 < j ? disassemble_byte(indata, i + 2) : 0;

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
