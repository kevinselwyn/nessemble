#include <stdlib.h>
#include "../nessemble.h"

/**
 * .incrle pseudo instruction
 * @param {char *} string - Filename of binary
 */
void pseudo_incrle(char *string) {
    unsigned int i = 0, j = 0, l = 0;
    unsigned int byte = 0, last = 0, count = 0;
    unsigned int run[256], has_run = FALSE, run_index = 0;
    unsigned int bin_length = 0;
    char *path = NULL, *bin_data = NULL;

    if (get_fullpath(&path, string) != 0) {
        yyerror(_("Could not get full path of %s"), string);
        goto cleanup;
    }

    if (!path) {
        yyerror(_("Could not get full path of %s"), string);
        goto cleanup;
    }

    if (load_file(&bin_data, &bin_length, path) != RETURN_OK) {
        yyerror(_("Could not read `%s`"), path);
        goto cleanup;
    }

    while (i < (unsigned int)bin_length) {
        byte = (unsigned int)bin_data[i] & 0xFF;
        last = i;
        count = 0;

        while (i < (unsigned int)bin_length && (unsigned int)bin_data[i] == byte) {
            count++;
            i++;
        }

        if (count > 2) {
            if (has_run == TRUE) {
                write_byte(0x80 + run_index);

                for (j = 0, l = run_index; j < l; j++) {
                    write_byte(run[j]);
                }
            }

            if (count > 0x7F) {
                while (count > 0x7F) {
                    write_byte(0x7F);
                    write_byte(byte);

                    count -= 0x7F;
                }
            }

            write_byte(count);
            write_byte(byte);

            has_run = FALSE;
        } else {
            if (has_run == FALSE) {
                run_index = 0;

                for (j = last, l = i; j < l; j++) {
                    run[run_index++] = (unsigned int)bin_data[j];
                }

                has_run = TRUE;
            } else {
                if (run_index > 0x7C) {
                    write_byte(0x80 + run_index);

                    for (j = 0, l = run_index; j < l; j++) {
                        write_byte(run[j]);
                    }

                    has_run = FALSE;
                } else {
                    for (j = last, l = i; j < l; j++) {
                        run[run_index++] = (unsigned int)bin_data[j];
                    }

                    has_run = TRUE;
                }
            }
        }
    }

    if (has_run == TRUE) {
        write_byte(0x80 + run_index);

        for (j = 0, l = run_index; j < l; j++) {
            write_byte(run[j]);
        }
    }

    write_byte(0xFF);

cleanup:
    nessemble_free(path);
    nessemble_free(bin_data);
}
