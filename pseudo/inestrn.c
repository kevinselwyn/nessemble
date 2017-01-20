#include <stdlib.h>
#include "../nessemble.h"

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

    if (!path) {
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
