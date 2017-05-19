#include <string.h>
#include "../nessemble.h"

#define USE_INTERFACE 1
#include "../third-party/tinyscheme-1.41/scheme.h"
#include "../third-party/tinyscheme-1.41/scheme-private.h"
#include "../third-party/tinyscheme-1.41/init.h"

size_t scm_length = 0;
char scm_output[1024];
char scm_error[1024];
unsigned int scm_rc;
unsigned int has_error;

pointer display(scheme *sc, pointer args) {
    if (args != sc->NIL) {
        if (sc->vptr->is_string(sc->vptr->pair_car(args)))  {
            char *str = sc->vptr->string_value(sc->vptr->pair_car(args));
            sprintf(scm_output+scm_length, "%s", str);
            scm_length += strlen(str);
        }

        if (sc->vptr->is_number(sc->vptr->pair_car(args)))  {
            unsigned int num = sc->vptr->ivalue(sc->vptr->pair_car(args));
            scm_output[scm_length++] = (char)(num % 256);
        }
    }

    return sc->NIL;
}

pointer error_hook(scheme *sc, pointer args) {
    memset(scm_error, '\0', 1024);
    sprintf(scm_error, "%s", sc->vptr->string_value(sc->vptr->pair_car(args)));
    has_error = TRUE;

    return sc->NIL;
}

pointer exit_hook(scheme *sc, pointer args) {
    if (args != sc->NIL) {
        if (sc->vptr->is_number(sc->vptr->pair_car(args)))  {
            scm_rc = sc->vptr->ivalue(sc->vptr->pair_car(args));
        }

        if (sc->vptr->is_string(sc->vptr->pair_car(sc->vptr->pair_cdr(args))))  {
            memset(scm_error, '\0', 1024);
            sprintf(scm_error, "%s", sc->vptr->string_value(sc->vptr->pair_car(sc->vptr->pair_cdr(args))));
            has_error = TRUE;
        }
    }

    return sc->NIL;
}

unsigned int scripting_scm(char *exec, char **output) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int exec_len = 0;
    char *exec_data = NULL;
    char scm_string[65536];
    scheme *sc;

    // init global vars
    scm_rc = 0;
    has_error = FALSE;

    // init scheme
    sc = scheme_init_new();

    // load file
    if (load_file(&exec_data, &exec_len, exec) != RETURN_OK) {
        yyerror(_("Could not load file `%s`"), exec);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // clear output
    scm_length = 0;
    memset(scm_output, '\0', 1024);

    // load init.scm
    scheme_load_string(sc, (char *)init_scm);

    // define display
    sc->vptr->scheme_define(
        sc,
        sc->global_env,
        sc->vptr->mk_symbol(sc, "display"),
        sc->vptr->mk_foreign_func(sc, display));

    sc->vptr->scheme_define(
        sc,
        sc->global_env,
        sc->vptr->mk_symbol(sc, "exit"),
        sc->vptr->mk_foreign_func(sc, exit_hook));

    sc->vptr->scheme_define(
        sc,
        sc->global_env,
        sc->vptr->mk_symbol(sc, "*error-hook*"),
        sc->vptr->mk_foreign_func(sc, error_hook));

    // load custom scm
    scheme_load_string(sc, exec_data);

    if (has_error == TRUE) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    // assemble custom scm call
    strcpy(scm_string, "(custom (list ");

    for (i = 0, l = length_ints; i < l; i++) {
        sprintf(scm_string+strlen(scm_string), "%d ", ints[i]);
    }

    scm_string[strlen(scm_string)-1] = ')';
    strcat(scm_string, " (list ");

    for (i = 0, l = length_texts; i < l; i++) {
        sprintf(scm_string+strlen(scm_string), "%s ", texts[i]);
    }

    scm_string[strlen(scm_string)-1] = ')';
    strcat(scm_string, ")");

    // call custom scm
    scheme_load_string(sc, scm_string);

    if (has_error == TRUE || scm_rc != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    // write bytes
    for (i = 0, l = (unsigned int)scm_length; i < l; i++) {
        write_byte((unsigned int)scm_output[i] & 0xFF);
    }

cleanup:
    if (has_error == TRUE) {
        *output = (char *)nessemble_malloc(sizeof(char) * (strlen(scm_error) + 1));
        strcpy(*output, scm_error);
    }

    scheme_deinit(sc);

    nessemble_free(sc);
    nessemble_free(exec_data);

    return rc;
}
