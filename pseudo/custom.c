#include <string.h>
#include "../nessemble.h"

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    char *exec = NULL, *ext = NULL;

    if (pseudo_parse(&exec, pseudo) != RETURN_OK) {
        yyerror(_("Unknown custom pseudo-instruction `%s`"), pseudo);
        goto cleanup;
    }

    if (!exec) {
        goto cleanup;
    }

    if (parse_extension(&ext, exec) != RETURN_OK) {
        yyerror(_("Unknown extension `%s`"), pseudo);
        goto cleanup;
    }

    if (file_exists(exec) == FALSE) {
        yyerror(_("Command for custom pseudo-instruction `%s` does not exist"), pseudo);
        goto cleanup;
    }

    if (ext != NULL && strcmp(ext, "js") == 0) {
        if (scripting_js(exec) != RETURN_OK) {
            goto cleanup;
        }
#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    } else if (ext != NULL && strcmp(ext, "py") == 0) {
        if (scripting_py(exec) != RETURN_OK) {
            goto cleanup;
        }
    } else if (ext != NULL && strcmp(ext, "lua") == 0) {
        if (scripting_lua(exec) != RETURN_OK) {
            goto cleanup;
        }
    } else if (ext != NULL && strcmp(ext, "so") == 0) {
        if (scripting_so(exec) != RETURN_OK) {
            goto cleanup;
        }
#endif /* !IS_WINDOWS && !IS_JAVSCRIPT */
    } else {
        if (scripting_cmd(exec) != RETURN_OK) {
            yyerror(_("Command for custom pseudo-instruction `%s` failed"), pseudo);
            goto cleanup;
        }
    }

cleanup:
    nessemble_free(exec);
    nessemble_free(ext);

    length_ints = 0;
}
