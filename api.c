#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nessemble.h"

static unsigned int api_endpoint(char **url, unsigned int num, ...) {
    unsigned int rc = RETURN_EPERM;
    unsigned int i = 0, l = 0, length = 0;
    char *registry = NULL, *output = NULL;
    va_list argptr;

    if ((rc = get_registry(&registry)) != RETURN_OK) {
        goto cleanup;
    }

    length += (unsigned int)strlen(registry);

    va_start(argptr, num);

    for (i = 0, l = num; i < l; i++) {
        length += (unsigned int)strlen(va_arg(argptr, char *)) + 1;
    }

    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    sprintf(output, "%s", registry);

    va_start(argptr, num);

    for (i = 0, l = num; i < l; i++) {
        sprintf(output+strlen(output), "/%s", va_arg(argptr, char *));
    }

    va_end(argptr);

cleanup:
    *url = output;

    nessemble_free(registry);

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
