#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nessemble.h"

static unsigned int api_endpoint(char **url, unsigned int num, ...) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, l = 0, length = 0;
    size_t registry_length = 0, arg_length = 0;
    char *arg = NULL, *registry = NULL, *output = NULL;
    va_list argptr;

    if ((rc = get_registry(&registry)) != RETURN_OK) {
        goto cleanup;
    }

    if (!registry) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    registry_length = strlen(registry);
    length += (unsigned int)registry_length;

    va_start(argptr, num);

    for (i = 0, l = num; i < l; i++) {
        length += (unsigned int)strlen(va_arg(argptr, char *)) + 1;
    }

    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    if (snprintf(output, registry_length+1, "%s", registry) != (int)(registry_length)) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    va_start(argptr, num);

    if (num > 0) {
        for (i = 0, l = num; i < l; i++) {
            arg = va_arg(argptr, char *);
            arg_length = strlen(arg);

            if (arg[0] == '.') {
                if (snprintf(output+strlen(output), (arg_length+1), "%s", arg) != (int)(arg_length)) {
                    rc = RETURN_EPERM;
                    goto cleanup;
                }
            } else {
                if (snprintf(output+strlen(output), (arg_length+2), "/%s", arg) != (int)(arg_length+1)) {
                    rc = RETURN_EPERM;
                    goto cleanup;
                }
            }
        }
    } else {
        if (snprintf(output+strlen(output), 2, "/") != 1) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    }

    va_end(argptr);

cleanup:
    *url = output;

    return rc;
}

unsigned int api_user(char **url, char *endpoint) {
    unsigned int rc = RETURN_OK;

    if ((rc = api_endpoint(&*url, 2, "user", endpoint)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int api_lib(char **url, char *lib) {
    unsigned int rc = RETURN_OK;
    char *version_lib = NULL, *version = NULL, *at = NULL;

    if ((at = strstr(lib, "@")) == NULL) {
        if ((rc = api_endpoint(&*url, 2, "package", lib)) != RETURN_OK) {
            goto cleanup;
        }

        goto cleanup;
    }

    at[0] = '\0';
    version_lib = lib;
    version = version_lib+((at-version_lib)+1);

    if ((rc = api_endpoint(&*url, 3, "package", version_lib, version)) != RETURN_OK) {
        goto cleanup;
    }

    at[0] = '@';

cleanup:
    return rc;
}

unsigned int api_search(char **url, char *term) {
    unsigned int rc = RETURN_OK, term_count = 0;

    term_count = (unsigned int)(term != NULL && term[0] != '.' ? 2 : 0);

    if ((rc = api_endpoint(&*url, term_count, "search", term)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int api_reference(char **url, char *endpoint) {
    unsigned int rc = RETURN_OK;

    if ((rc = api_endpoint(&*url, 2, "reference", endpoint)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}
