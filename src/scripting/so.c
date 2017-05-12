#include <string.h>
#include "../nessemble.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#include <dlfcn.h>
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

#ifdef IS_WINDOWS
#include <windows.h>
#endif /* IS_WINDOWS */

unsigned int scripting_so(char *exec) {
    unsigned int rc = RETURN_OK;

#ifndef IS_JAVASCRIPT
    void *handle = NULL;
    int (*custom)(char **, size_t *, unsigned int[], int, char *[], int);
    unsigned int i = 0, l = 0;
    size_t return_len = 0;
    char *return_str = NULL;

#ifndef IS_WINDOWS
    handle = dlopen(exec, RTLD_LAZY);

    if (!handle) {
        yyerror(_("Could not open `%s`"), exec);
    }

    custom = dlsym(handle, "custom");

    if (dlerror() != NULL) {
        yyerror(_("Could not fetch function `custom`"));
    }
#else /* IS_WINDOWS */
    HMODULE dll_handle = LoadLibrary(exec);

    if (!dll_handle) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    FARPROC dll_func = GetProcAddress(dll_handle, "custom");

    custom = dll_func;
#endif /* IS_WINDOWS */

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
#ifndef IS_WINDOWS
    if (handle) {
        UNUSED(dlclose(handle));
    }
#endif /* IS_WINDOWS */

    nessemble_free(return_str);

#endif /* IS_JAVASCRIPT */
    return rc;
}
