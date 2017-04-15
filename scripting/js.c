#include "../nessemble.h"
#include "../third-party/duktape/duktape.h"

unsigned int scripting_js(char *exec) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int exec_len = 0;
    size_t return_len = 0;
    char *exec_data = NULL, *return_str = NULL,  *error_str = NULL;

    duk_context *ctx = duk_create_heap_default();

    if (load_file(&exec_data, &exec_len, exec) != RETURN_OK) {
        yyerror(_("Could not load file `%s`"), exec);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    duk_push_string(ctx, exec_data);
    duk_eval(ctx);

    duk_get_global_string(ctx, "add_string");

    for (i = 0, l = length_texts; i < l; i++) {
        duk_push_string(ctx, texts[i]);

        if (duk_pcall(ctx, 1) != 0) {
            error_str = strchr(duk_safe_to_string(ctx, -1), ':');
            yyerror(error_str+2);
        }
    }

    duk_get_global_string(ctx, "custom");

    for (i = 0, l = length_ints; i < l; i++) {
        duk_push_int(ctx, ints[i]);
    }

    if (duk_pcall(ctx, length_ints) != 0) {
        error_str = strchr(duk_safe_to_string(ctx, -1), ':');
        yyerror(error_str+2);
    }

    return_str = (char *)duk_get_string(ctx, -1);
    return_len = (size_t)duk_get_length(ctx, -1);

    for (i = 0, l = (unsigned int)return_len; i < l; i++) {
        write_byte((unsigned int)return_str[i] & 0xFF);
    }

cleanup:
    duk_destroy_heap(ctx);
    nessemble_free(exec_data);

    return rc;
}
