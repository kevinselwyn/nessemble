#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"

unsigned int if_depth = 0;
unsigned int if_active = FALSE;
unsigned int if_type = IF_IFDEF;
unsigned int if_cond = FALSE;
char *if_label = NULL;

/**
 * .ascii pseudo instruction
 * @param {char *} string - ASCII string
 * @param {int} offset - Offset
 */
void pseudo_ascii(char *string, int offset) {
    unsigned int i = 0, l = 0;
    size_t length = 0;

    length = strlen(string);

    for (i = 1, l = (unsigned int)length - 1; i < l; i++) {
        write_byte((unsigned int)string[i] + offset);
    }
}

/**
 * .chr[0-9]+ pseudo instruction
 * @param {int} index - CHR bank index
 */
void pseudo_chr(unsigned int index) {
    segment_type = SEGMENT_CHR;

    chr_index = index;
}

/**
 * .db pseudo instruction
 */
void pseudo_db() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = (unsigned int)length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}

/**
 * .defchr pseudo instruction
 */
void pseudo_defchr() {
    unsigned int i = 0, j = 0, k = 0, l = 0, loops = 2, digit = 0;
    unsigned int byte = 0;

    if (length_ints != 8) {
        yyerror("Too few arguments. %d provided, need 8", length_ints);
    }

    while (loops != 0) {
        for (i = 0, j = 8; i < j; i++) {
            byte = 0;

            for (k = 0, l = 8; k < l; k++) {
                digit = (ints[i] / power(10, (int)(l - k - 1))) % 10;
                digit = (digit >> (loops - 1)) & 0x01;

                byte |= (digit << (l - k - 1));
            }

            write_byte(byte);
        }

        loops -= 1;
    }

    length_ints = 0;
}

/**
 * .dw pseudo instruction
 */
void pseudo_dw() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
        write_byte((ints[i] >> 8) & 0xFF);
    }

    length_ints = 0;
}

/**
 * .else pseudo instruction
 */
void pseudo_else() {
    if (if_type == IF_IFDEF) {
        if_type = IF_IFNDEF;
    } else {
        if_type = IF_IFDEF;
    }
}

/**
 * .endif pseudo instruction
 */
void pseudo_endif() {
    if_depth--;

    if (if_depth == 0) {
        if_active = FALSE;
    }
}

/**
 * .hibytes pseudo instruction
 */
void pseudo_hibytes() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte((ints[i] >> 8) & 0xFF);
    }

    length_ints = 0;
}

/**
 * .if pseudo instruction
 */
void pseudo_if(unsigned int cond) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IF;
    if_cond = cond;
}

/**
 * .ifdef pseudo instruction
 */
void pseudo_ifdef(char *label) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IFDEF;
    if_label = label;
}

/**
 * .ifndef pseudo instruction
 */
void pseudo_ifndef(char *label) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IFNDEF;
    if_label = label;
}

/**
 * .ineschr pseudo instruction
 * @param {int} value - Number of 8KB CHR banks
 */
void pseudo_ineschr(unsigned int value) {
    flags |= FLAG_NES;

    ines.chr = value;
}

/**
 * .inesmap pseudo instruction
 * @param {int} value - Mapper ID
 */
void pseudo_inesmap(unsigned int value) {
    flags |= FLAG_NES;

    ines.map = value;
}

/**
 * .inesmir pseudo instruction
 * @param {int} value - Mirroring value
 */
void pseudo_inesmir(unsigned int value) {
    flags |= FLAG_NES;

    ines.mir = value;
}

/**
 * .inesprg pseudo instruction
 * @param {int} value - Number of 16KB PRG banks
 */
void pseudo_inesprg(unsigned int value) {
    flags |= FLAG_NES;

    ines.prg = value;
}

/**
 * .inestrn pseudo instruction
 * @param {char *} string - Filename of trainer
 */
void pseudo_inestrn(char *string) {
    char *path = NULL;

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    flags |= FLAG_NES;

    ines.trn = 1;

    include_file_push(path);

cleanup:
    if (path) {
        free(path);
    }
}

/**
 * .incbin pseudo instruction
 * @param {char *} string - Filename of binary
 */
void pseudo_incbin(char *string, int offset, int limit) {
    unsigned int i = 0, l = 0;
    size_t bin_length = 0;
    char *path = NULL, *bin_data = NULL;
    FILE *incbin = NULL;

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    incbin = fopen(path, "r");

    if (!incbin) {
        yyerror("Could not include %s", string);
        goto cleanup;
    }

    if (fseek(incbin, 0, SEEK_END) != 0) {
        yyerror("Seek error");
        goto cleanup;
    }

    bin_length = (size_t)ftell(incbin);

    if (fseek(incbin, 0, SEEK_SET) != 0) {
        yyerror("Seek error");
        goto cleanup;
    }

    if (bin_length == 0) {
        goto cleanup;
    }

    bin_data = (char *)malloc(sizeof(char) * (bin_length + 1));

    if (!bin_data) {
        yyerror("Memory error");
        goto cleanup;
    }

    if (fread(bin_data, 1, bin_length, incbin) != bin_length) {
        yyerror("Could not read %s", string);
        goto cleanup;
    }

    if (limit == -1) {
        limit = (int)bin_length;
    }

    for (i = (unsigned int)offset, l = (unsigned int)limit; i < l; i++) {
        write_byte((unsigned int)bin_data[i] & 0xFF);
    }

cleanup:
    if (incbin) {
        (void)fclose(incbin);
    }

    if (path) {
        free(path);
    }

    if (bin_data) {
        free(bin_data);
    }
}

/**
 * .include pseudo instruction
 * @param {char *} string - Filename to include
 */
void pseudo_include(char *string) {
    char *path = NULL;

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    include_file_push(path);

cleanup:
    if (path) {
        free(path);
    }
}

/**
 * .lobytes pseudo instruction
 */
void pseudo_lobytes() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}

/**
 * .org pseudo instruction
 * @param {int} address - Organization address
 */
void pseudo_org(unsigned int address) {
    // TODO: add check for too high an address

    if (is_segment_prg() == TRUE) {
        if (ines.prg < 2) {
            prg_offsets[prg_index] = address - 0xC000;
        } else {
            prg_offsets[prg_index] = address - 0x8000;
        }
    }

    if (is_segment_chr() == TRUE) {
        chr_offsets[chr_index] = address;
    }
}

/**
 * .out pseudo instruction
 * @param {char *} string - Text to output
 */
void pseudo_out(char *string) {
    size_t length = 0;

    if (pass == 2) {
        length = strlen(string);
        string[length - 1] = '\0';

        fprintf(stderr, "%s\n", string+1);
    }
}

/**
 * .prg[0-9]+ pseudo instruction
 * @param {int} index - PRG bank index
 */
void pseudo_prg(unsigned int index) {
    segment_type = SEGMENT_PRG;

    prg_index = index;
}

/**
 * .rsset pseudo instruction
 * @param {int} address - Variable start address
 */
void pseudo_rsset(unsigned int address) {
    rsset = address;
}

/**
 * .rs pseudo instruction
 * @param {char *} label - Variable label
 * @param {int} size - Variable size
 */
void pseudo_rs(char *label, int size) {
    add_symbol(label, rsset, SYMBOL_RS);

    rsset += size;
}

/**
 * .segment pseudo instruction
 * @param {char *} string - Segment name
 */
void pseudo_segment(char *string) {
    size_t length = 0;

    length = strlen(string);

    strcpy(segment, string+1);
    segment[length-2] = '\0';

    if (length-2 > 3 && strncmp(segment, "PRG", 3) == 0) {
        pseudo_prg((unsigned int)dec2int(segment+3));
    }

    if (length-2 > 3 && strncmp(segment, "CHR", 3) == 0) {
        pseudo_chr((unsigned int)dec2int(segment+3));
    }
}
