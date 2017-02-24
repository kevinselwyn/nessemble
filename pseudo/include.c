#include <stdlib.h>
#include "../nessemble.h"

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

    if (!path) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    include_file_push(path);

cleanup:
    nessemble_free(path);
}
