#include <stdlib.h>
#include "../nessemble.h"

/**
 * .incbin pseudo instruction
 * @param {char *} string - Filename of binary
 */
void pseudo_incbin(char *string, int offset, int limit) {
    unsigned int i = 0, l = 0;
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

    if (limit == -1) {
        limit = (int)bin_length;
    }

    for (i = (unsigned int)offset, l = (unsigned int)limit; i < l; i++) {
        write_byte((unsigned int)bin_data[i] & 0xFF);
    }

cleanup:
    nessemble_free(path);
    nessemble_free(bin_data);
}
