#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "init.h"

char buffer[BUF_SIZE];

char *get_line(char *prompt) {
    printf("%s", prompt);
    return fgets(buffer, BUF_SIZE, stdin);
}

unsigned int init() {
    unsigned int rc = 0, i = 0, l = 0;
    unsigned int input_prg = 0, input_chr = 0, input_mapper = 0, input_mirroring = 0;
    size_t length = 0;
    char *input_filename = NULL;
    FILE *output = NULL;

    while (get_line("Filename: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        input_filename = strdup(buffer);
        break;
    }

    while (get_line("PRG Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        sscanf(buffer, "%d", &input_prg);
        break;
    }

    while (get_line("CHR Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        sscanf(buffer, "%d", &input_chr);
        break;
    }


    while (get_line("Mapper (0-255): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        sscanf(buffer, "%d", &input_mapper);
        break;
    }

    while (get_line("Mirroring (0-15): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        sscanf(buffer, "%d", &input_mirroring);
        break;
    }

    output = fopen(input_filename, "w+");

    if (!output) {
        fprintf(stderr, "Could not open `%s`\n", input_filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    fprintf(output, ".inesprg %d\n.ineschr %d\n.inesmap %d\n.inesmir %d\n", input_prg, input_chr, input_mapper % 0xFF, input_mirroring % 0x0F);

    for (i = 0, l = input_prg; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.prg %d\n", i);

        if (i == 0) {
            init_asm[init_asm_len-1] = '\0';
            fprintf(output, "\n%s\n", init_asm);
        }
    }

    for (i = 0, l = input_chr; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.chr %d\n", i);
    }

cleanup:
    if (input_filename) {
        free(input_filename);
    }

    if (output) {
        (void)fclose(output);
    }

    return rc;
}
