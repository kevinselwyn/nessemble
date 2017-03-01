#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nessemble.h"
#include "png.h"

#define _GNU_SOURCE
#include <stdlib.h>
int fileno(FILE *file);

static unsigned int reassemblable = FALSE;

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
        fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, inesprg, inesprg);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".inesprg %u\n", inesprg);
}

static void disassemble_ineschr(FILE *outfile, unsigned int offset, unsigned int ineschr) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, ineschr, ineschr);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".ineschr %u\n", ineschr);
}

static void disassemble_inesmir(FILE *outfile, unsigned int offset, unsigned int arg0) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, arg0, arg0);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".inesmir %u\n", arg0 & 0x01);
}

static void disassemble_inesmap(FILE *outfile, unsigned int offset, unsigned int arg0, unsigned int arg1) {
    if (reassemblable == FALSE) {
        fprintf(outfile, "%04X | XXXX | %02X       | .db $%02X ; ", offset, arg1, arg1);
    } else {
        fprintf(outfile, "    ");
    }

    fprintf(outfile, ".inesmap %u\n", (arg0 >> 4) | (arg1 & 0xF0));
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

static void disassemble_fill(FILE *outfile, unsigned int value, unsigned int count) {
    fprintf(outfile, "    .fill %d, $%02X\n", count, value);
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

static unsigned int disassemble_chr(char *data, unsigned int length, char *filename) {
    unsigned int rc = RETURN_OK;
    unsigned int x = 0, y = 0, byte1 = 0, byte2 = 0, bit1 = 0, bit2 = 0, pixel = 0;
    unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
    unsigned int pixels[128 * 256];

    for (i = 0, j = length; i < j; i += 0x10) {
        for (k = 0, l = 0x08; k < l; k++) {
            byte1 = data[i+k];
            byte2 = data[i+k+0x08];

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
        goto cleanup;
    }

cleanup:
    return rc;
}

int disassemble(char *input, char *output, char *listname) {
    int rc = RETURN_OK;
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int opcode_id = 0, arg0 = 0, arg1 = 0, symbol_found = FALSE, skip_symbol = FALSE;
    unsigned int fill_index = 0, fill_count = 0, fill_offset = 0, fill_done = FALSE;
    unsigned int offset = 0, end_offset = 0, inesprg = 0, ineschr = 0, inestrn = FALSE, ines_header = FALSE;
    unsigned int insize = 0;
    char *indata = NULL, *chr_filename = NULL;
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
            fprintf(outfile, "0000 | XXXX | 4E 45 53 | .ascii \"NES\"\n");
            fprintf(outfile, "0003 | XXXX | 1A       | .db $1A\n");
        }

        inesprg = disassemble_byte(indata, 4);
        disassemble_inesprg(outfile, 4, inesprg);

        ineschr = disassemble_byte(indata, 5);
        disassemble_ineschr(outfile, 5, ineschr);

        arg0 = disassemble_byte(indata, 6);
        inestrn = (unsigned int)((arg0 & 0x04) != 0 ? TRUE : FALSE);
        disassemble_inesmir(outfile, 6, arg0);
        disassemble_inesmap(outfile, 7, arg0, disassemble_byte(indata, 7));

        if (reassemblable == FALSE) {
            disassemble_db_header(outfile, 8, disassemble_byte(indata, 8));
            disassemble_db_header(outfile, 9, disassemble_byte(indata, 9));
            disassemble_db_header(outfile, 10, disassemble_byte(indata, 10));

            for (i = 11, j = 16; i < j; i++) {
                disassemble_db_header(outfile, i, disassemble_byte(indata, i));
            }
        }

        i = 0x10;
    }

    ines_header = (unsigned int)(inesprg != 0 || ineschr != 0 ? TRUE : FALSE);

    if (ines_header == TRUE && reassemblable == TRUE) {
        fprintf(outfile, "\n");
    }

    if ((rc = input_list(listname)) != RETURN_OK) {
        goto cleanup;
    }

    j = (unsigned int)insize;

    if (reassemblable == TRUE) {
        j -= (ineschr * BANK_CHR);
    }

    for (; i < j; i++) {
        opcode_id = disassemble_byte(indata, i);
        offset = disassemble_offset(i, inesprg, ineschr, inestrn);

        if (ines_header == TRUE) {
            if (reassemblable == FALSE) {
                fprintf(outfile, "%04X | ", i);
            } else {
                if ((i - 0x10) % BANK_PRG == 0) {
                    fprintf(outfile, "\n    .prg %d\n\n", (i - 0x10) / BANK_PRG);
                }
            }
        }

        if (reassemblable == TRUE) {
            symbol_found = FALSE;
            skip_symbol = FALSE;

            for (k = 0, l = symbol_index; k < l; k++) {
                if (symbols[k].type == SYMBOL_LABEL && symbols[k].value == offset) {
                    fprintf(outfile, "\n%s: ; %04X\n", symbols[k].name, symbols[k].value);
                    symbol_found = TRUE;
                }
            }

            if (symbol_found == FALSE) {
                end_offset = disassemble_offset(i + opcodes[opcode_id].length, inesprg, ineschr, inestrn);

                for (k = 0, l = symbol_index; k < l; k++) {
                    if (symbols[k].value > offset && symbols[k].value < end_offset) {
                        skip_symbol = TRUE;
                    }
                }
            }
        }

        if ((opcodes[opcode_id].meta & META_UNDOCUMENTED) == 0) {
            if (i + (opcodes[opcode_id].length - 1) >= insize) {
                if (reassemblable == FALSE) {
                    disassemble_db(outfile, offset, opcode_id);
                } else {
                    fill_count = 0;
                    fill_index = i;

                    while (++fill_index < j + 1 && disassemble_byte(indata, fill_index) == opcode_id) {
                        fill_offset = disassemble_offset(fill_index, inesprg, ineschr, inestrn);
                        fill_done = FALSE;

                        for (k = 0, l = symbol_index; k < l; k++) {
                            if (symbols[k].value == fill_offset) {
                                fill_done = TRUE;
                            }
                        }

                        fill_count++;

                        if (fill_done == TRUE) {
                            break;
                        }
                    }

                    if (fill_count >= 0x10) {
                        disassemble_fill(outfile, opcode_id, fill_count);

                        i += fill_count - 1;
                        continue;
                    } else {
                        disassemble_db(outfile, offset, opcode_id);
                    }
                }

                i += 1;
                continue;
            }

            if (reassemblable == TRUE && skip_symbol == TRUE) {
                disassemble_db(outfile, offset, opcode_id);
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
                if (reassemblable == FALSE) {
                    disassemble_db(outfile, offset, opcode_id);
                } else {
                    fill_count = 0;
                    fill_index = i;

                    while (++fill_index < j + 1 && disassemble_byte(indata, fill_index) == opcode_id) {
                        fill_offset = disassemble_offset(fill_index, inesprg, ineschr, inestrn);
                        fill_done = FALSE;

                        for (k = 0, l = symbol_index; k < l; k++) {
                            if (symbols[k].value == fill_offset) {
                                fill_done = TRUE;
                            }
                        }

                        fill_count++;

                        if (fill_done == TRUE) {
                            break;
                        }
                    }

                    if (fill_count >= 0x10) {
                        disassemble_fill(outfile, opcode_id, fill_count);

                        i += fill_count - 1;
                        continue;
                    } else {
                        disassemble_db(outfile, offset, opcode_id);
                    }
                }

                break;
            }
        } else {
            if (reassemblable == FALSE) {
                disassemble_db(outfile, offset, opcode_id);
            } else {
                fill_count = 0;
                fill_index = i;

                while (++fill_index < j + 1 && disassemble_byte(indata, fill_index) == opcode_id) {
                    fill_offset = disassemble_offset(fill_index, inesprg, ineschr, inestrn);
                    fill_done = FALSE;

                    for (k = 0, l = symbol_index; k < l; k++) {
                        if (symbols[k].value == fill_offset) {
                            fill_done = TRUE;
                        }
                    }

                    fill_count++;

                    if (fill_done == TRUE) {
                        break;
                    }
                }

                if (fill_count >= 0x10) {
                    disassemble_fill(outfile, opcode_id, fill_count);

                    i += fill_count - 1;
                    continue;
                } else {
                    disassemble_db(outfile, offset, opcode_id);
                }
            }
        }
    }

    // output chr
    if (isatty(fileno(outfile)) == 1 || strcmp(output, "/dev/stdout") == 0) {
        goto cleanup;
    }

    chr_filename = (char *)nessemble_malloc(sizeof(char) * (strlen(output) + 11));

    for (i = 0, l = ineschr; i < l; i++) {
        sprintf(chr_filename, "%s-chr%d.png", output, i);

        if ((rc = disassemble_chr(indata+((insize - (ineschr * BANK_CHR)) + (i * BANK_CHR)), BANK_CHR, chr_filename)) != RETURN_OK) {
            goto cleanup;
        }

        fprintf(outfile, "\n    .chr %d\n\n", i);
        fprintf(outfile, "    .incpng \"%s\"\n", chr_filename);
    }

cleanup:
    nessemble_free(indata);
    nessemble_free(chr_filename);

    if (outfile) {
        (void)fclose(outfile);
    }

    return rc;
}
