#include <stdio.h>
#include <stdlib.h>
#include <json/json.h>
#include "nessemble.h"

#include <string.h>
char *strstr(const char *haystack, const char *needle);
#define _GNU_SOURCE
#include <string.h>
char *strcasestr(const char *haystack, const char *needle);

#define MIMETYPE_JSON "application/json"

unsigned int get_json(char **value, char *key, char *url) {
    unsigned int rc = RETURN_OK, http_code = 0;
    size_t text_length = 0, string_length = 0;
    char *text = NULL, *output = NULL, *string_value = NULL, *k_val = NULL;
    json_object *jobj;
    enum json_type type;
    struct json_tokener *tok;
    struct json_object *v_val;

    http_code = get_request(&text, &text_length, url, MIMETYPE_JSON);

    switch (http_code) {
    case 503:
        fprintf(stderr, "Could not reach the registry\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 404:
        fprintf(stderr, "Library does not exist\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 200:
    default:
        break;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    tok = json_tokener_new();

    if (!tok) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    jobj = json_tokener_parse_ex(tok, text, text_length);

    if (!jobj) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (struct lh_entry *entry = json_object_get_object(jobj)->head; ({ if (entry) { k_val = (char *)entry->k; v_val = (struct json_object *)entry->v; } ; entry; }); entry = entry->next) {
        if (strcmp(key, k_val) == 0) {
            type = json_object_get_type(v_val);

            if (type != json_type_string) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            string_value = (char *)json_object_get_string(v_val);
            string_length = strlen(string_value);
        }
    }

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

    if (tok) {
        json_tokener_free(tok);
    }

    return rc;
}

unsigned int get_json_search(char *url, char *term) {
    unsigned int rc = RETURN_OK, i = 0, j = 0, k = 0, l = 0;
    size_t text_length = 0;
    char *text = NULL, *k_val = NULL;
    json_object *jobj;
    enum json_type type;
    struct json_tokener *tok;
    struct json_object *v_val, *results;

    switch (get_request(&text, &text_length, url, MIMETYPE_JSON)) {
    case 503:
        fprintf(stderr, "Could not reach the registry\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 404:
        fprintf(stderr, "Library does not exist\n");

        rc = RETURN_EPERM;
        goto cleanup;
        break;
    case 200:
    default:
        break;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    tok = json_tokener_new();

    if (!tok) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    jobj = json_tokener_parse_ex(tok, text, text_length);

    if (!jobj) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (struct lh_entry *entry = json_object_get_object(jobj)->head; ({ if (entry) { k_val = (char *)entry->k; v_val = (struct json_object *)entry->v; } ; entry; }); entry = entry->next) {
        if (strcmp("results", k_val) == 0 || strcmp("libraries", k_val) == 0) {
            type = json_object_get_type(v_val);

            if (type != json_type_array) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            results = json_object_object_get(jobj, k_val);

            for (i = 0, j = json_object_array_length(results); i < j; i++) {
                int name_index = 0, description_index = 0, term_length = 0;
                json_object *result = NULL, *name = NULL, *description = NULL;
                char *name_text = NULL, *description_text = NULL;

                result = json_object_array_get_idx(results, (size_t)i);
                type = json_object_get_type(result);

                if (type != json_type_object) {
                    continue;
                }

                name = json_object_object_get(result, "name");
                type = json_object_get_type(name);

                if (type != json_type_string) {
                    continue;
                }

                description = json_object_object_get(result, "description");
                type = json_object_get_type(description);

                if (type != json_type_string) {
                    continue;
                }

                name_text = (char *)json_object_get_string(name);
                description_text = (char *)json_object_get_string(description);

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
        }
    }

/*
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
*/

cleanup:
    if (text) {
        free(text);
    }

    return rc;
}
