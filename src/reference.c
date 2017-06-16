#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nessemble.h"
#include "http.h"

unsigned int reference(unsigned int terms, ...) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int url_length = 0, endpoint_length = 0;
    char *arg = NULL, *url = NULL, *endpoint = NULL, *text = NULL;
    http_t *request;
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

    if (!url) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    url_length = (unsigned int)strlen(url);

    if (url[url_length - 1] == '/') {
        url[url_length - 1] = '\0';
    }

    http_init(request);

    if ((rc = http_header(request, "Accept", MIMETYPE_TEXT)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_get(request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (request->status_code != 200) {
        if (request->status_code != 500) {
            for (i = 0, l = endpoint_length; i < l; i++) {
                if (endpoint[i] == '/') {
                    endpoint[i] = ' ';
                }
            }

            if (endpoint_length > 0) {
                error_program_log(_("Could not find info for `%s`"), endpoint);
            } else {
                error_program_log(_("Could not find info"));
            }
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (terms >= 2) {
        if ((rc = pager_buffer(request->response_body)) != RETURN_OK) {
            goto cleanup;
        }
    } else {
        printf("%s", request->response_body);
    }

    http_release(request);

cleanup:
    nessemble_free(endpoint);
    nessemble_free(url);
    nessemble_free(text);

    return rc;
}
