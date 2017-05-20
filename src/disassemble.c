#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "nessemble.h"
#include "png.h"

enum {
    DISASSEMBLE_OUTPUT = 0x00,
    DISASSEMBLE_TRAINER = 0x01
};

static unsigned int reassemblable = FALSE;
static unsigned int disassemble_ines_header = FALSE;
static unsigned int disassemble_inesprg = 0;
static unsigned int disassemble_ineschr = 0;
static unsigned int disassemble_inestrn = FALSE;
static unsigned int disassemble_bank = 0;

static unsigned int disassemble_pass = 0;
static size_t disassemble_outsizes[2] = { 0, 0 };
static char *disassemble_outdatas[2] = { NULL, NULL };
static FILE *disassemble_outfiles[2] = { NULL, NULL };

static void disassemble_write(unsigned int outfile, char *fmt, ...) {
    char line[1024];

    va_list argptr;
    va_start(argptr, fmt);

    UNUSED(vsprintf(line, fmt, argptr));

    if (disassemble_pass == 1) {
        disassemble_outsizes[outfile] += strlen(line);
    }

    if (disassemble_pass == 2) {
        strcat(disassemble_outdatas[outfile], line);
    }

    va_end(argptr);
}

static unsigned int disassemble_offset(unsigned int offset) {
    if (disassemble_inesprg == 0 && disassemble_ineschr == 0) {
        return offset;
    }

    offset -= 0x10;

    if (disassemble_inestrn == TRUE) {
        if (offset >= 0x200) {
            offset -= 0x200;
        } else {
            return offset;
        }
    }

    if (offset >= disassemble_inesprg * BANK_PRG) {
        offset -= disassemble_inesprg * BANK_PRG;
    } else {
        if (disassemble_inesprg == 1) {
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

static void disassemble_inesprg_byte(unsigned int outfile, unsigned int offset, unsigned int inesprg) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, inesprg, inesprg);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".inesprg %u\n", inesprg);
}

static void disassemble_ineschr_byte(unsigned int outfile, unsigned int offset, unsigned int ineschr) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, ineschr, ineschr);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".ineschr %u\n", ineschr);
}

static void disassemble_inesmir_byte(unsigned int outfile, unsigned int offset, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, arg0, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".inesmir %u\n", arg0 & 0x01);
}

static void disassemble_inesmap_byte(unsigned int outfile, unsigned int offset, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, arg1, arg1);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".inesmap %u\n", (arg0 >> 4) | (arg1 & 0xF0));
}

static void disassemble_db_header(unsigned int outfile, unsigned int offset, unsigned int value) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | XXXX | %02X       | ", offset, value);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".db $%02X\n", value);
}

static void disassemble_db(unsigned int outfile, unsigned int offset, unsigned int value) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X       | ", offset, value);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, ".db $%02X", value);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_implied(unsigned int outfile, unsigned int offset, unsigned int opcode_id) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X       | ", offset, opcode_id);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s", opcodes[opcode_id].mnemonic);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_accumulator(unsigned int outfile, unsigned int offset, unsigned int opcode_id) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X       | ", offset, opcode_id);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s A", opcodes[opcode_id].mnemonic);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_relative(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int value) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s $%04X", opcodes[opcode_id].mnemonic, value);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_immediate(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s #$%02X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_zeropage(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s <$%02X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_zeropage_x(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s <$%02X, X", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_zeropage_y(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s <$%02X, Y", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_absolute(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s $%04X", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_absolute_x(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s $%04X, X", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_absolute_y(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s $%04X, Y", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_indirect(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X %02X | ", offset, opcode_id, arg0, arg1);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s [$%04X]", opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_indirect_x(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s [$%02X, X]", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static void disassemble_indirect_y(unsigned int outfile, unsigned int offset, unsigned int opcode_id, unsigned int arg0) {
    if (reassemblable == FALSE) {
        disassemble_write(outfile, "%04X | %02X %02X    | ", offset, opcode_id, arg0);
    } else {
        disassemble_write(outfile, "    ");
    }

    disassemble_write(outfile, "%s [$%02X], Y", opcodes[opcode_id].mnemonic, arg0);

    if (reassemblable == TRUE && (offset % 0x100) == 0) {
        disassemble_write(outfile, " ; %04X", offset);
    }

    disassemble_write(outfile, "\n");
}

static unsigned int disassemble_data(unsigned int outfile, char *data, unsigned int length, unsigned int start) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int arg0 = 0, arg1 = 0, opcode_id = 0;
    unsigned int offset = 0, end_offset = 0;
    unsigned int symbol_found = FALSE, symbol_skip = FALSE;

    for (i = 0, j = length; i < j; i++) {
        opcode_id = disassemble_byte(data, i);
        offset = disassemble_offset(start + i);

        if (disassemble_ines_header == TRUE && reassemblable == FALSE) {
            disassemble_write(outfile, "%04X | ", start + i);
        }

        if (reassemblable == TRUE) {
            symbol_found = FALSE;
            symbol_skip = FALSE;

            for (k = 0, l = symbol_index; k < l; k++) {
                if (symbols[k].type == SYMBOL_LABEL && symbols[k].value == offset && symbols[k].bank == disassemble_bank) {
                    disassemble_write(outfile, "\n%s: ; %04X\n", symbols[k].name, symbols[k].value);
                    symbol_found = TRUE;
                }
            }

            if (symbol_found == FALSE) {
                end_offset = disassemble_offset(i + start + opcodes[opcode_id].length);

                for (k = 0, l = symbol_index; k < l; k++) {
                    if (symbols[k].type == SYMBOL_LABEL && symbols[k].value > offset && symbols[k].value < end_offset) {
                        symbol_skip = TRUE;
                    }
                }
            }
        }

        if (is_flag_undocumented() == FALSE && (opcodes[opcode_id].meta & META_UNDOCUMENTED) != 0) {
            disassemble_db(outfile, offset, opcode_id);
            continue;
        }

        if (i + (opcodes[opcode_id].length - 1) >= length) {
            disassemble_db(outfile, offset, opcode_id);
            continue;
        }

        if (reassemblable == TRUE && symbol_skip == TRUE) {
            disassemble_db(outfile, offset, opcode_id);
            continue;
        }

        arg0 = i + 1 < j ? disassemble_byte(data, i + 1) : 0;
        arg1 = i + 2 < j ? disassemble_byte(data, i + 2) : 0;

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
    }

    return rc;
}

static unsigned int disassemble_chr(char *data, unsigned int length, char *filename) {
    unsigned int rc = RETURN_OK;
    unsigned int x = 0, y = 0, byte1 = 0, byte2 = 0, bit1 = 0, bit2 = 0, pixel = 0;
    unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
    unsigned int *pixels = NULL;

    pixels = (unsigned int *)nessemble_malloc(sizeof(unsigned int) * (128 * 256));

    for (i = 0, j = length; i < j; i += 0x10) {
        for (k = 0, l = 0x08; k < l; k++) {
            byte1 = (unsigned int)data[i+k];
            byte2 = (unsigned int)data[i+k+0x08];

            for (m = 0, n = 8; m < n; m++) {
                bit1 = (byte1 >> (7 - m)) & 1;
                bit2 = (byte2 >> (7 - m)) & 1;
                pixel = bit1 | (bit2 << 1);

                pixels[((x * 8) + m) + (((y * 8) + k) * 128)] = pixel;
            }
        }

        if (++x * 8 >= 128) {
            x = 0;
            y++;
        }
    }

    if ((rc = write_png(pixels, 128, 256, filename)) != RETURN_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    nessemble_free(pixels);

    return rc;
}

unsigned int disassemble_main(char *indata, unsigned int insize, char *output, char *listname) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, j = 0, l = 0;
    unsigned int arg0 = 0, prg_counter = 0, outfile = 0;
    char *trn_filename = NULL, *chr_filename = NULL;
    FILE *trnfile = NULL;

    disassemble_pass++;
    outfile = DISASSEMBLE_OUTPUT;

    reassemblable = is_flag_reassemble();

    if (strncmp(indata, "NES", 3) == 0) {
        if (reassemblable == FALSE) {
            disassemble_write(outfile, "0000 | XXXX | 4E 45 53 | .ascii \"NES\"\n");
            disassemble_write(outfile, "0003 | XXXX | 1A       | .db $1A\n");
        }

        disassemble_inesprg = disassemble_byte(indata, 4);
        disassemble_inesprg_byte(outfile, 4, disassemble_inesprg);

        disassemble_ineschr = disassemble_byte(indata, 5);
        disassemble_ineschr_byte(outfile, 5, disassemble_ineschr);

        arg0 = disassemble_byte(indata, 6);
        disassemble_inestrn = (unsigned int)((arg0 & 0x04) != 0 ? TRUE : FALSE);
        disassemble_inesmir_byte(outfile, 6, arg0);
        disassemble_inesmap_byte(outfile, 7, arg0, disassemble_byte(indata, 7));

        if (reassemblable == FALSE) {
            disassemble_db_header(outfile, 8, disassemble_byte(indata, 8));
            disassemble_db_header(outfile, 9, disassemble_byte(indata, 9));
            disassemble_db_header(outfile, 10, disassemble_byte(indata, 10));

            for (i = 11, j = 16; i < j; i++) {
                disassemble_db_header(outfile, i, disassemble_byte(indata, i));
            }
        } else {
            if (is_stdout(output) == FALSE && disassemble_inestrn == TRUE && reassemblable == TRUE) {
                trn_filename = (char *)nessemble_malloc(sizeof(char) * (strlen(output) + 13));
                sprintf(trn_filename, "%s-trainer.asm", output);

                trnfile = fopen(trn_filename, "w+");
                disassemble_outfiles[DISASSEMBLE_TRAINER] = trnfile;

                disassemble_data(DISASSEMBLE_TRAINER, indata+0x10, 0x200, i);
                disassemble_write(outfile, "    .inestrn \"%s\"", trn_filename);
            }
        }

        i = 0x10;
    }

    disassemble_ines_header = (unsigned int)(disassemble_inesprg != 0 || disassemble_ineschr != 0 ? TRUE : FALSE);

    if (disassemble_ines_header == TRUE && reassemblable == TRUE) {
        disassemble_write(outfile, "\n");
    }

    if ((rc = input_list(listname)) != RETURN_OK) {
        goto cleanup;
    }

    if (disassemble_ines_header == TRUE) {
        i = 0x10;

        if (disassemble_inestrn == TRUE) {
            if (reassemblable == FALSE) {
                disassemble_data(outfile, indata+i, 0x200, i);
            }

            i += 0x200;
        }

        for (j = (unsigned int)insize - (disassemble_ineschr * BANK_CHR); i < j; i += BANK_PRG) {
            if (disassemble_ines_header == TRUE && reassemblable == TRUE) {
                disassemble_bank = prg_counter;
                disassemble_write(outfile, "\n    .prg %u\n\n", prg_counter++);
            }

            disassemble_data(outfile, indata+i, BANK_PRG, i);
        }
    } else {
        disassemble_data(outfile, indata, insize, 0);
    }

    /* output chr */
    if (is_stdout(output) == TRUE) {
        goto cleanup;
    }

    chr_filename = (char *)nessemble_malloc(sizeof(char) * (strlen(output) + 11));

    if (disassemble_ines_header == TRUE && reassemblable == TRUE) {
        for (i = 0, l = disassemble_ineschr; i < l; i++) {
            sprintf(chr_filename, "%s-chr%u.png", output, i);

            if ((rc = disassemble_chr(indata+((insize - (disassemble_ineschr * BANK_CHR)) + (i * BANK_CHR)), BANK_CHR, chr_filename)) != RETURN_OK) {
                goto cleanup;
            }

            disassemble_write(outfile, "\n    .chr %u\n\n", i);
            disassemble_write(outfile, "    .incpng \"%s\"\n", chr_filename);
        }
    }

cleanup:
    nessemble_free(trn_filename);
    nessemble_free(chr_filename);

    return rc;
}

unsigned int disassemble(char *input, char *output, char *listname) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, l = 0;
    unsigned int insize = 0;
    char *indata = NULL;
    FILE *outfile = NULL;

    if ((rc = load_file(&indata, &insize, input)) != RETURN_OK) {
        goto cleanup;
    }

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
        error_program_log(_("Could not open `%s`"), output);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    disassemble_outfiles[DISASSEMBLE_OUTPUT] = outfile;

    if ((rc = disassemble_main(indata, insize, output, listname)) != RETURN_OK) {
        goto cleanup;
    }

    for (i = 0, l = 2; i < l; i++) {
        if (disassemble_outfiles[i] != NULL) {
            disassemble_outdatas[i] = (char *)nessemble_malloc(sizeof(char) * (disassemble_outsizes[i] + 1));
        }
    }

    if ((rc = disassemble_main(indata, insize, output, listname)) != RETURN_OK) {
        goto cleanup;
    }

    for (i = 0, l = 2; i < l; i++) {
        if (disassemble_outfiles[i] != NULL) {
            if (fwrite(disassemble_outdatas[i], 1, disassemble_outsizes[i], disassemble_outfiles[i]) != disassemble_outsizes[i]) {
                rc = RETURN_EPERM;
                goto cleanup;
            }
        }
    }

cleanup:
    nessemble_free(indata);

    for (i = 0, l = 2; i < l; i++) {
        nessemble_free(disassemble_outdatas[i]);
        nessemble_fclose(disassemble_outfiles[i]);
    }

    return rc;
}
