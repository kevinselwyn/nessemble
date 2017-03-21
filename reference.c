#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nessemble.h"

unsigned int reference(unsigned int terms, ...) {
    unsigned int rc = RETURN_OK, http_code = 0, i = 0, l = 0;
    unsigned int url_length = 0, endpoint_length = 0, text_length = 0;
    char *arg = NULL, *url = NULL, *endpoint = NULL, *text = NULL;
    va_list argptr;

    va_start(argptr, terms);

    for (i = 0, l = terms; i < l; i++) {
        arg = va_arg(argptr, char *);
        endpoint_length += (unsigned int)strlen(arg) + 1;
    }

    endpoint = (char *)nessemble_malloc(sizeof(char) * (endpoint_length + 1));

    va_start(argptr, terms);

    for (i = 0, l = terms; i < l; i++) {
        arg = va_arg(argptr, char *);

        if (i == 0) {
            sprintf(endpoint+strlen(endpoint), "%s", arg);
        } else {
            sprintf(endpoint+strlen(endpoint), "/%s", arg);
        }
    }

    va_end(argptr);

    if ((rc = api_reference(&url, endpoint)) != RETURN_OK) {
        goto cleanup;
    }

    url_length = (unsigned int)strlen(url);

    if (url[url_length - 1] == '/') {
        url[url_length - 1] = '\0';
    }

    http_code = get_request(&text, &text_length, url, 0, MIMETYPE_TEXT);

    if (http_code != 200) {
        if (http_code != 500) {
            for (i = 0, l = endpoint_length; i < l; i++) {
                if (endpoint[i] == '/') {
                    endpoint[i] = ' ';
                }
            }

            if (endpoint_length > 0) {
                error_program_log("Could not find info for '%s'", endpoint);
            } else {
                error_program_log("Could not find info");
            }
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (terms >= 2) {
        if ((rc = pager_buffer(text)) != RETURN_OK) {
            goto cleanup;
        }
    } else {
        printf("%s\n", text);
    }

cleanup:
    return rc;
}
