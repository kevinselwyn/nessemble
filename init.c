#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "init.h"

#define INIT_BUF_SIZE 256

static char *get_line(char **buffer, char *prompt) {
    printf("%s", prompt);
    return fgets(*buffer, INIT_BUF_SIZE, stdin);
}

unsigned int init() {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    int input_prg = 0, input_chr = 0, input_mapper = 0, input_mirroring = 0;
    size_t length = 0;
    char *buffer = NULL, *input_filename = NULL;
    FILE *output = NULL;

    buffer = (char *)nessemble_malloc(sizeof(char) * INIT_BUF_SIZE);

    while (get_line(&buffer, "Filename: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        input_filename = nessemble_strdup(buffer);
        break;
    }

    while (get_line(&buffer, "PRG Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_prg) != 1) {
            continue;
        }

        if (input_prg < 0) {
            printf("Choose a positive number of CHR banks\n");
            continue;
        }

        break;
    }

    while (get_line(&buffer, "CHR Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_chr) != 1) {
            continue;
        }

        if (input_chr < 0) {
            printf("Choose a positive number of CHR banks\n");
            continue;
        }

        break;
    }


    while (get_line(&buffer, "Mapper (0-255): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_mapper) != 1) {
            continue;
        }

        if (input_mapper < 0 || input_mapper > 0xFF) {
            printf("Choose a mapper between 0-255\n");
            continue;
        }

        break;
    }

    while (get_line(&buffer, "Mirroring (0-15): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%d", &input_mirroring) != 1) {
            continue;
        }

        if (input_mirroring < 0 || input_mirroring > 0x0F) {
            printf("Choose a mirroring between 0-15\n");
            continue;
        }

        break;
    }

    if (!input_filename) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    output = fopen(input_filename, "w+");

    if (!output) {
        fprintf(stderr, "Could not open `%s`\n", input_filename);

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

    printf("Created `%s`\n", input_filename);

cleanup:
    nessemble_free(input_filename);
    nessemble_free(buffer);

    if (output) {
        (void)fclose(output);
    }

    return rc;
}
