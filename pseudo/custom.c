#include <string.h>
#include "../nessemble.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#define CUSTOM_SCRIPT_COUNT 3
#else /* !IS_WINDOWS && !IS_JAVASCRIPT */
#ifdef IS_WINDOWS
#define CUSTOM_SCRIPT_COUNT 2
#else /* IS_WINDOWS */
#define CUSTOM_SCRIPT_COUNT 1
#endif /* IS_WINDOWS */
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

struct custom_script {
    char *ext;
    unsigned int (*func)(char *);
};

struct custom_script custom_scripts[CUSTOM_SCRIPT_COUNT] = {
    { "js",  &scripting_js  },
#ifndef IS_JAVASCRIPT
    { "lua", &scripting_lua },
#ifndef IS_WINDOWS
    { "so",  &scripting_so  }
#endif /* IS_WINDOWS */
#endif /* IS_JAVASCRIPT */
};

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    char *exec = NULL, *ext = NULL;
    unsigned int i = 0, l = 0, found = FALSE;

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

    if (ext != NULL) {
        for (i = 0, l = CUSTOM_SCRIPT_COUNT; i < l; i++) {
            if (strcmp(custom_scripts[i].ext, ext) == 0) {
                found = TRUE;

                if ((*custom_scripts[i].func)(exec) != RETURN_OK) {
                    goto cleanup;
                }
            }
        }
    }

    if (found == FALSE) {
        if (scripting_cmd(exec) != RETURN_OK) {
            yyerror(_("Command for custom pseudo-instruction `%s` failed"), pseudo);
            goto cleanup;
        }
    }

cleanup:
    nessemble_free(exec);
    nessemble_free(ext);

    length_ints = 0;

    for (i = 0, l = length_texts; i < l; i++) {
        nessemble_free(texts[i]);
    }

    length_texts = 0;
}
