#include <string.h>
#include "../nessemble.h"

#if defined(SCRIPTING_JAVASCRIPT) && SCRIPTING_JAVASCRIPT != 0
#define CUSTOM_SCRIPT_JS 1
#else
#define CUSTOM_SCRIPT_JS 0
#endif

#if defined(SCRIPTING_SCHEME) && SCRIPTING_SCHEME != 0
#define CUSTOM_SCRIPT_SCM 1
#else
#define CUSTOM_SCRIPT_SCM 0
#endif

#if defined(SCRIPTING_LUA) && SCRIPTING_LUA != 0
#define CUSTOM_SCRIPT_LUA 1
#else
#define CUSTOM_SCRIPT_LUA 0
#endif

#ifdef IS_JAVSCRIPT
#define CUSTOM_SCRIPT_COUNT 0
#else
#define CUSTOM_SCRIPT_COUNT (CUSTOM_SCRIPT_JS+CUSTOM_SCRIPT_SCM+CUSTOM_SCRIPT_LUA+1)
#endif

struct custom_script {
    char *ext;
    unsigned int (*func)(char *, char **);
};

struct custom_script custom_scripts[CUSTOM_SCRIPT_COUNT] = {
#if CUSTOM_SCRIPT_JS == 1
    { "js",  &scripting_js  },
#endif
#if CUSTOM_SCRIPT_SCM == 1
    { "scm", &scripting_scm },
#endif
#if CUSTOM_SCRIPT_LUA == 1
    { "lua", &scripting_lua },
#endif
#ifndef IS_JAVASCRIPT
#ifndef IS_WINDOWS
    { "so",  &scripting_so  }
#else /* IS_WINDOWS */
    { "dll", &scripting_so }
#endif /* IS_WINDOWS */
#endif /* IS_JAVASCRIPT */
};

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    char *exec = NULL, *ext = NULL, *output = NULL;
    unsigned int i = 0, l = 0, found = FALSE, has_error = FALSE;

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

                if ((*custom_scripts[i].func)(exec, &output) != RETURN_OK) {
                    has_error = TRUE;
                    goto cleanup;
                }
            }
        }
    }

    if (found == FALSE) {
#if CUSTOM_SCRIPT_JS == 0
        if (strcmp(ext, "js") == 0) {
            yyerror(_("Javascript scripting is disabled"));
            goto cleanup;
        }
#endif

#if CUSTOM_SCRIPT_SCM == 0
        if (strcmp(ext, "scm") == 0) {
            yyerror(_("Scheme scripting is disabled"));
            goto cleanup;
        }
#endif

#if CUSTOM_SCRIPT_LUA == 0
        if (strcmp(ext, "lua") == 0) {
            yyerror(_("Lua scripting is disabled"));
            goto cleanup;
        }
#endif

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

    if (has_error == TRUE) {
        error(output);
        nessemble_free(output);
    }
}
