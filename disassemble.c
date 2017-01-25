#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

int disassemble(char *input, char *output) {
    int rc = RETURN_OK;
    unsigned int i = 0, l = 0, opcode_id = 0, arg0 = 0, arg1 = 0;
    size_t insize = 0;
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
        i = 0x10;

        fprintf(outfile, "0000 | 4E 45 53 | .ascii \"NES\"\n");
        fprintf(outfile, "0003 | 1A       | .db $1A\n");

        arg0 = (unsigned int)indata[4] & 0xFF;
        fprintf(outfile, "0004 | %02X       | .db $%02X ; .inesprg %u\n", arg0, arg0, arg0);

        arg0 = (unsigned int)indata[5] & 0xFF;
        fprintf(outfile, "0005 | %02X       | .db $%02X ; .ineschr %u\n", arg0, arg0, arg0);

        arg0 = (unsigned int)indata[6] & 0xFF;
        arg1 = (unsigned int)indata[7] & 0xFF;
        fprintf(outfile, "0006 | %02X       | .db $%02X ; .inesmir %u\n", arg0, arg0, arg0 & 0x01);
        fprintf(outfile, "0007 | %02X       | .db $%02X ; .inesmap %u\n", arg1, arg1, (arg0 >> 4) | (arg1 & 0xF0));

        arg0 = (unsigned int)indata[8] & 0xFF;
        fprintf(outfile, "0008 | %02X       | .db $%02X\n", arg0, arg0);

        arg0 = (unsigned int)indata[9] & 0xFF;
        fprintf(outfile, "0009 | %02X       | .db $%02X\n", arg0, arg0);

        arg0 = (unsigned int)indata[10] & 0xFF;
        fprintf(outfile, "000A | %02X       | .db $%02X\n", arg0, arg0);

        for (i = 11, l = 16; i < l; i++) {
            fprintf(outfile, "%04X | %02X       | .db $%02X\n", i, 0, 0);
        }

        i = 16;
    }

    for (l = (unsigned int)insize; i < l; i++) {
        opcode_id = (unsigned int)indata[i] & 0xFF;

        if ((opcodes[opcode_id].meta & META_UNDOCUMENTED) == 0) {
            if (i + (opcodes[opcode_id].length - 1) >= insize) {
                fprintf(outfile, "%04X | %02X       | .db $%02X\n", i, opcode_id, opcode_id);
                i += 1;
                continue;
            }

            switch (opcodes[opcode_id].mode) {
            case MODE_IMPLIED:
                fprintf(outfile, "%04X | %02X       | %s\n", i, opcode_id, opcodes[opcode_id].mnemonic);
                break;
            case MODE_ACCUMULATOR:
                fprintf(outfile, "%04X | %02X       | %s A\n", i, opcode_id, opcodes[opcode_id].mnemonic);
                break;
            case MODE_RELATIVE:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                if (arg0 >= 0x80) {
                    fprintf(outfile, "%04X | %02X %02X    | %s $%04X\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, i - (0xFF - arg0 - 1));
                } else {
                    fprintf(outfile, "%04X | %02X %02X    | %s $%04X\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, i + arg0);
                }
                i++;
                break;
            case MODE_IMMEDIATE:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s #$%02X\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            case MODE_ZEROPAGE:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s <$%02X\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            case MODE_ZEROPAGE_X:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s <$%02X, X\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            case MODE_ZEROPAGE_Y:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s <$%02X, Y\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            case MODE_ABSOLUTE:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                arg1 = (unsigned int)indata[i+2] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X\n", i, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
                i += 2;
                break;
            case MODE_ABSOLUTE_X:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                arg1 = (unsigned int)indata[i+2] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X, X\n", i, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
                i += 2;
                break;
            case MODE_ABSOLUTE_Y:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                arg1 = (unsigned int)indata[i+2] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X %02X | %s $%04X, Y\n", i, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
                i += 2;
                break;
            case MODE_INDIRECT:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                arg1 = (unsigned int)indata[i+2] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X %02X | %s ($%04X)\n", i, opcode_id, arg0, arg1, opcodes[opcode_id].mnemonic, (arg1 << 8) | arg0);
                i += 2;
                break;
            case MODE_INDIRECT_X:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s ($%02X, X)\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            case MODE_INDIRECT_Y:
                arg0 = (unsigned int)indata[i+1] & 0xFF;
                fprintf(outfile, "%04X | %02X %02X    | %s ($%02X), Y\n", i, opcode_id, arg0, opcodes[opcode_id].mnemonic, arg0);
                i++;
                break;
            default:
                fprintf(outfile, "%04X | %02X       | .db $%02X\n", i, opcode_id, opcode_id);
                break;
            }
        } else {
            fprintf(outfile, "%04X | %02X       | .db $%02X\n", i, opcode_id, opcode_id);
        }
    }

cleanup:
    if (indata) {
        free(indata);
    }

    if (outfile) {
        (void)fclose(outfile);
    }

    return rc;
}
