#include <string.h>
#include "../nessemble.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#include <dlfcn.h>
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

unsigned int scripting_so(char *exec) {
    unsigned int rc = RETURN_OK;

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    void *handle = NULL;
    int (*custom)(char **, size_t *, unsigned int *, int, char *[], int);
    unsigned int i = 0, l = 0;
    size_t return_len = 0;
    char *return_str = NULL;

    handle = dlopen(exec, RTLD_LAZY);

    if (!handle) {
        yyerror(_("Could not open `%s`"), exec);
    }

    custom = dlsym(handle, "custom");

    if (dlerror() != NULL) {
        yyerror(_("Could not fetch function `custom`"));
    }

    if ((*custom)(&return_str, &return_len, ints, length_ints, texts, length_texts) != 0) {
        yyerror(return_str);
    }

    if (!return_str) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (return_len >= 1) {
        for (i = 0, l = (unsigned int)return_len; i < l; i++) {
            write_byte((unsigned int)return_str[i] & 0xFF);
        }
    }

cleanup:
    if (handle) {
        UNUSED(dlclose(handle));
    }

    nessemble_free(return_str);

#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */
    return rc;
}
