#include <string.h>
#include <jansson.h>
#include <curl/curl.h>
#include "nessemble.h"
#include "json.h"

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream) {
    struct write_result *result = (struct write_result *)stream;

    if (result->pos + size * nmemb >= BUFFER_SIZE - 1) {
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

unsigned int get_request(char **request, char *url) {
    unsigned int rc = RETURN_OK;
    long code = 0;
    char *data = NULL;
    struct curl_slist *headers = NULL;
    CURL *curl = NULL;
    CURLcode status;

    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl = curl_easy_init();

    if (!curl) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data = (char *)malloc(sizeof(char) * BUFFER_SIZE);

    if (!data) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);

    if (status != 0) {
        fprintf(stderr, "Could not reach the registry\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

    if (code != 200) {
        fprintf(stderr, "Library does not exist\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[write_result.pos] = '\0';

    *request = data;

cleanup:
    return rc;
}

unsigned int get_json(char **value, char *key, char *url) {
    unsigned int rc = RETURN_OK;
    size_t string_length = 0;
    char *text = NULL, *string_value = NULL;
    json_t *root = NULL, *data = NULL;
    json_error_t error;

    if (get_request(&text, url) != RETURN_OK) {
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

    *value = (char *)malloc(sizeof(char) * (string_length) + 1);
    strcpy(*value, string_value);

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
    char *text = NULL;
    json_t *root = NULL, *results = NULL;
    json_error_t error;

    if (get_request(&text, url) != RETURN_OK) {
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

    for (i = 0, j = json_array_size(results); i < j; i++) {
        int name_index = 0, description_index = 0, term_length = 0;
        json_t *result = NULL, *name = NULL, *description = NULL;
        char *name_text = NULL, *description_text = NULL;

        result = json_array_get(results, i);

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
