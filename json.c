#include <jansson.h>
#include "nessemble.h"

#include <string.h>
char *strstr(const char *haystack, const char *needle);
#define _GNU_SOURCE
#include <string.h>
char *strcasestr(const char *haystack, const char *needle);

#define MIMETYPE_JSON "application/json"

unsigned int get_json(char **value, char *key, char *url) {
    unsigned int rc = RETURN_OK;
    size_t text_length = 0, string_length = 0;
    char *text = NULL, *output = NULL, *string_value = NULL;
    json_t *root = NULL, *data = NULL;
    json_error_t error;

    if (get_request(&text, &text_length, url, MIMETYPE_JSON) != RETURN_OK) {
        fprintf(stderr, "Library does not exist\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    root = json_loads(text, 0, &error);

    if (!root) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data = json_object_get(root, key);

    if (!json_is_string(data)) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    string_value = (char *)json_string_value(data);
    string_length = strlen(string_value);

    if (string_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    output = (char *)malloc(sizeof(char) * (string_length) + 1);

    if (!output) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    strcpy(output, string_value);

    *value = output;

cleanup:
    if (text) {
        free(text);
    }

    if (root) {
        json_decref(root);
    }

    return rc;
}

unsigned int get_json_search(char *url, char *term) {
    unsigned int rc = RETURN_OK, i = 0, j = 0, k = 0, l = 0;
    size_t text_length = 0;
    char *text = NULL;
    json_t *root = NULL, *results = NULL;
    json_error_t error;

    if (get_request(&text, &text_length, url, MIMETYPE_JSON) != RETURN_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    root = json_loads(text, 0, &error);

    if (!root) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    results = json_object_get(root, "results");

    if (!json_is_array(results)) {
        results = json_object_get(root, "libraries");

        if (!json_is_array(results)) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    }

    for (i = 0, j = (unsigned int)json_array_size(results); i < j; i++) {
        int name_index = 0, description_index = 0, term_length = 0;
        json_t *result = NULL, *name = NULL, *description = NULL;
        char *name_text = NULL, *description_text = NULL;

        result = json_array_get(results, (size_t)i);

        if (!json_is_object(result)) {
            continue;
        }

        name = json_object_get(result, "name");

        if (!json_is_string(name)) {
            continue;
        }

        description = json_object_get(result, "description");

        if (!json_is_string(description)) {
            continue;
        }

        name_text = (char *)json_string_value(name);
        description_text = (char *)json_string_value(description);

        term_length = (int)strlen(term);
        name_index = strcasestr(name_text, term) - name_text;
        description_index = strcasestr(description_text, term) - description_text;

        if (name_index >= 0) {
            for (k = 0, l = (unsigned int)strlen(name_text); k < l; k++) {
                if (k == (unsigned int)name_index) {
                    printf("\e[1m");
                }

                printf("%c", name_text[k]);

                if (k + 1 == (unsigned int)(name_index + term_length)) {
                    printf("\e[0m");
                }
            }
        } else {
            printf("%s", name_text);
        }

        printf(" - ");

        if (description_index >= 0) {
            for (k = 0, l = (unsigned int)strlen(description_text); k < l; k++) {
                if (k == (unsigned int)description_index) {
                    printf("\e[1m");
                }

                printf("%c", description_text[k]);

                if (k + 1 == (unsigned int)(description_index + term_length)) {
                    printf("\e[0m");
                }
            }
        } else {
            printf("%s", description_text);
        }

        printf("\n");
    }

cleanup:
    if (text) {
        free(text);
    }

    if (root) {
        json_decref(root);
    }

    return rc;
}
