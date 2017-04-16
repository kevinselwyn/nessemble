#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nessemble.h"
#include "init.h"

unsigned int init(int optc, char *optv[]) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    int input_prg = -1, input_chr = -1, input_mapper = -1, input_mirroring = -1;
    size_t length = 0;
    char answer = '\0';
    char *buffer = NULL, *input_filename = NULL;
    FILE *output = NULL;

    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    if (optc >= 1) {
        input_filename = nessemble_strdup(optv[0]);
    }

    while (!input_filename && get_line(&buffer, _("Filename: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        input_filename = nessemble_strdup(buffer);
        break;
    }

    if (optc >= 2) {
        input_prg = atoi(optv[1]);
    }

    while (input_prg == -1 && get_line(&buffer, _("PRG Banks: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_prg) != 1) {
            continue;
        }

        if (input_prg < 0) {
            printf("%s\n", _("Choose a positive number of CHR banks"));
            continue;
        }

        break;
    }

    if (optc >= 3) {
        input_chr = atoi(optv[2]);
    }

    while (input_chr == -1 && get_line(&buffer, _("CHR Banks: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_chr) != 1) {
            continue;
        }

        if (input_chr < 0) {
            printf("%s\n", _("Choose a positive number of CHR banks"));
            continue;
        }

        break;
    }

    if (optc >= 4) {
        input_mapper = atoi(optv[3]);
    }

    while (input_mapper == -1 && get_line(&buffer, _("Mapper (0-255): ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_mapper) != 1) {
            continue;
        }

        if (input_mapper < 0 || input_mapper > 0xFF) {
            printf("%s\n", _("Choose a mapper between 0-255"));
            continue;
        }

        break;
    }

    if (optc >= 5) {
        input_mirroring = atoi(optv[4]);
    }

    while (input_mirroring == -1 && get_line(&buffer, _("Mirroring (0-15): ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_mirroring) != 1) {
            continue;
        }

        if (input_mirroring < 0 || input_mirroring > 0x0F) {
            printf("%s\n", _("Choose a mirroring between 0-15"));
            continue;
        }

        break;
    }

    if (!input_filename) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (file_exists(input_filename) == TRUE) {
        while (answer != 'y' && answer != 'Y') {
            printf(_("`%s` already exists. Overwrite? [Yn] "), input_filename);

            if (scanf(" %c", &answer) != 1) {
                goto cleanup;
            }

            if (answer == 'n' || answer == 'N') {
                goto cleanup;
            }
        }
    }

    output = fopen(input_filename, "w+");

    if (!output) {
        error_program_log(_("Could not open `%s`"), input_filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    fprintf(output, ".inesprg %d\n.ineschr %d\n.inesmap %d\n.inesmir %d\n", input_prg, input_chr, input_mapper % 0xFF, input_mirroring % 0x0F);

    for (i = 0, l = (unsigned int)input_prg; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.prg %u\n", i);

        if (i == 0) {
            init_txt[init_txt_len-1] = '\0';
            fprintf(output, "\n%s\n", (char *)init_txt);
        }
    }

    for (i = 0, l = (unsigned int)input_chr; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.chr %u\n", i);
    }

    printf(_("Created `%s`"), input_filename);
    printf("\n");

cleanup:
    nessemble_free(input_filename);
    nessemble_free(buffer);
    nessemble_fclose(output);

    return rc;
}
