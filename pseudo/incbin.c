#include <stdlib.h>
#include "../nessemble.h"

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

    if (!path) {
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
