#include <string.h>
#include <stdlib.h>
#include "nessemble.h"

// ines
struct ines_header ines = { 1, 0, 0, 1, 0 };

/**
 * End parser pass
 */
void end_pass() {
    pass++;
}

/**
 * Get ROM index
 * @return {int} ROM index
 */
unsigned int get_rom_index() {
    unsigned int index = 0;

    if (is_segment_prg() == TRUE) {
        index = prg_offsets[prg_index] + (prg_index * BANK_PRG);
    }

    if (is_segment_chr() == TRUE) {
        index = chr_offsets[chr_index] + (ines.prg * BANK_PRG);

        if (chr_index > 0) {
            index += chr_index * BANK_CHR;
        }
    }

    return index;
}

/**
 * Get address offset
 * @return {int} PC offset
 */
unsigned int get_address_offset() {
    unsigned int offset = 0;

    if (is_flag_nes() == TRUE) {
        if (is_segment_prg() == TRUE) {
            if (ines.prg < 2) {
                offset = prg_offsets[prg_index] + (BANK_PRG * 3);
            } else {
                offset = prg_offsets[prg_index] + ((BANK_PRG * 2) + ((prg_index % 2) * BANK_PRG));
            }
        }
    } else {
        offset = prg_offsets[prg_index];
    }

    if (is_segment_chr() == TRUE) {
        offset = chr_offsets[chr_index] + (ines.prg * BANK_PRG);

        if (chr_index > 0) {
            offset += chr_index * BANK_CHR;
        }
    }

    return offset;
}

/**
 * Write byte
 * @param {unsigned int} byte - Byte to write
 */
void write_byte(unsigned int byte) {
    unsigned int offset = get_rom_index();

    if (if_active == TRUE) {
        if (if_cond[if_depth] == FALSE) {
            return;
        } else {
            if (if_depth - 1 != 0 && if_cond[if_depth - 1] == FALSE) {
                return;
            }
        }
    }

    if (ines.trn == 1) {
        if (pass == 2) {
            trainer[offset_trainer] = byte;
        }

        offset_trainer++;

        return;
    }

    if (is_flag_nes() == FALSE && (int)offset + 1 > offset_max) {
        offset_max = (int)offset + 1;
    }

    if (pass == 2 && offset < (unsigned int)offset_max) {
        rom[offset] = byte;
        coverage[offset] = TRUE;
    }

    if (is_segment_prg() == TRUE) {
        if (pass == 1) {
            if (prg_offsets[prg_index] >= BANK_PRG) {
                warning(_("Overflowing PRG Bank %d"), prg_index);
            }
        }

        prg_offsets[prg_index]++;
    }

    if (is_segment_chr() == TRUE) {
        if (pass == 1) {
            if (chr_offsets[chr_index] >= BANK_CHR) {
                warning(_("Overflowing CHR Bank %d"), prg_index);
            }
        }

        chr_offsets[chr_index]++;
    }
}

/**
 * Add symbol
 * @param {char *} name - Symbol name
 * @param {unsigned int} value - Symbol value
 * @param {unsigned int} type - Symbol type
 */
void add_symbol(char *name, unsigned int value, unsigned int type) {
    int symbol_id = -1;
    unsigned int bank = 0;

    if (pass == 1) {
        if (if_active == TRUE) {
            if (if_cond[if_depth] == FALSE) {
                return;
            } else {
                if (if_depth - 1 != 0 && if_cond[if_depth - 1] == FALSE) {
                    return;
                }
            }
        }

        bank = prg_index;
        symbol_id = get_symbol(name);

        if (symbol_id != -1) {
            symbols[symbol_id].value = value;
            symbols[symbol_id].bank = bank;
            symbols[symbol_id].type = type;
        } else {
            symbols[symbol_index].name = nessemble_strdup(name);
            symbols[symbol_index].value = value;
            symbols[symbol_index].bank = bank;
            symbols[symbol_index++].type = type;
        }
    }
}

/**
 * Get symbol
 * @param {char *} name - Symbol name
 * @return {int} Symbol ID
 */
int get_symbol(char *name) {
    int symbol_id = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = symbol_index; i < l; i++) {
        if (symbol_id == -1 && strcmp(symbols[i].name, name) == 0) {
            symbol_id = (int)i;
        }
    }

    if (pass == 2 && symbol_id != -1 && symbols[symbol_id].type == SYMBOL_UNDEFINED) {
        error(_("Symbol `%s` was not defined"), symbols[symbol_id].name);
    }

    return symbol_id;
}

/**
 * Add constant
 * @param {char *} name - Constant name
 * @param {int} value - Constant value
 */
void add_constant(char *name, unsigned int value) {
    add_symbol(name, value, SYMBOL_CONSTANT);
}

/**
 * Add label
 * @param {char *} name - Label name
 */
void add_label(char *name) {
    unsigned int offset = get_address_offset();

    add_symbol(name, offset, SYMBOL_LABEL);
}

/**
 * Has label test
 * @param {char *} name - Label name
 * @return {int} True if true, false if not
 */
int has_label(char *name) {
    unsigned int i = 0, l = 0;

    for (i = 0, l = symbol_index; i < l; i++) {
        if (strcmp(symbols[i].name, name) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

unsigned int pseudo_parse(char **exec, char *pseudo) {
    unsigned int rc = RETURN_OK, found = FALSE;
    unsigned int global_found = FALSE, global_check = FALSE;
    size_t pseudo_length = 0;
    char buffer[1024], *val = NULL, *output = NULL, *scripts = NULL;
    FILE *pseudo_file = NULL;

    pseudo_file = fopen(pseudoname, "r");

global_recheck:
    if (!pseudo_file) {
        rc = RETURN_EPERM;
        goto global_check;
    }

    pseudo_length = strlen(pseudo);

    while (fgets(buffer, 1024, pseudo_file) != NULL) {
        if (strncmp(buffer, pseudo, pseudo_length) == 0) {
            val = nessemble_strdup(buffer+(pseudo_length + 3));
            found = TRUE;
        }
    }

    if (global_check == TRUE) {
        if (found == FALSE) {
            goto cleanup;
        }

        goto global_continue;
    }

    if (found != TRUE) {
global_check:
        global_check = TRUE;

        if ((rc = get_home_path(&scripts, 3, "." PROGRAM_NAME, "scripts", "scripts.txt")) != RETURN_OK) {
            goto cleanup;
        }

        if (file_exists(scripts) == FALSE) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        nessemble_fclose(pseudo_file);
        pseudo_file = fopen(scripts, "r");
        nessemble_free(scripts);

        global_found = TRUE;

        goto global_recheck;
    }

global_continue:
    pseudo_length = strlen(val);

    if (val[pseudo_length - 1] == '\n') {
        val[pseudo_length - 1] = '\0';
        pseudo_length -= 1;
    }

    pseudo_length += strlen(cwd_path) + 1;

    if (global_found) {
        if ((rc = get_home_path(&output, 3, "." PROGRAM_NAME, "scripts", val))) {
            goto cleanup;
        }
    } else {
        output = (char *)nessemble_malloc(sizeof(char) * (pseudo_length + 1));
        sprintf(output, "%s" SEP "%s", cwd_path, val);
    }

cleanup:
    *exec = output;

    nessemble_free(val);
    nessemble_fclose(pseudo_file);

    return rc;
}
