#include <stdio.h>
#include <stdlib.h>
#include "nessemble.h"
#include "json.h"

#include <string.h>
char *strstr(const char *haystack, const char *needle);
#define _GNU_SOURCE
#include <string.h>
char *strcasestr(const char *haystack, const char *needle);

static unsigned int parse_json(struct json_token **json_tokens, char *json, unsigned int json_token_count) {
    unsigned int key = 0, type = 0, start = 0, end = 0, size = 0;
    int token_index = 0, i = 0, l = 0;
    struct json_token *tokens = NULL;

    tokens = (struct json_token *)nessemble_malloc(sizeof(struct json_token) * (json_token_count));

    l = (int)strlen(json);

    if (json[i++] != '{') {
        goto cleanup;
    }

    type = JSON_UNDEFINED;

    while (i < l) {
        if (i >= l) {
            break;
        }

        while (1 == TRUE) {
            if (i >= l) {
                break;
            }

            if (json[i] == '"') {
                type = JSON_STRING;
                break;
            }

            if ((int)json[i] >= 48 && (int)json[i] <= 57) {
                type = JSON_NUMBER;
                break;
            }

            if (json[i] == 't' || json[i] == 'f') {
                type = JSON_BOOLEAN;
                break;
            }

            if (json[i] == 'n') {
                type = JSON_NULL;
                break;
            }

            i++;
        }

        if (type == JSON_STRING) {
            i++;
        }

        start = (unsigned int)i;

        while (1 == TRUE) {
            if (i >= l) {
                break;
            }

            if (type == JSON_STRING) {
                if (json[i] == '"' && json[i-1] != '\\') {
                    break;
                }
            }

            if (type == JSON_NUMBER || type == JSON_BOOLEAN || type == JSON_NULL) {
                if (json[i] == ',' || json[i] == '}' || json[i] == '\n' || json[i] == ' ') {
                    break;
                }
            }

            i++;
        }

        end = (unsigned int)i;

        key = FALSE;

        while (1 == TRUE) {
            if (i >= l) {
                break;
            }

            if (json[i] == ':') {
                key = TRUE;
                break;
            }

            if (json[i] == '}') {
                key = FALSE;
                break;
            }

            if (json[i] == ',') {
                break;
            }

            i++;
        }

        tokens[token_index].key = key;
        tokens[token_index].type = type;
        tokens[token_index].start = start;
        tokens[token_index].end = end;
        tokens[token_index].size = 0;

        token_index++;
        i++;
    }

    size = 0;

    for (i = token_index - 1, l = 0; i >= l; --i) {
        if (tokens[i].key == FALSE) {
            size++;
        } else {
            tokens[i].size = size;
            size = 0;
        }
    }

cleanup:
    *json_tokens = tokens;

    return (unsigned int)token_index;
}

static unsigned int parse_text(char **output, char *text) {
    unsigned int rc = RETURN_OK;
    unsigned int length = 0, index = 0, i = 0, l = 0;
    char *parsed = NULL;

    length = (unsigned int)strlen(text);

    parsed = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    for (i = 0, l = length; i < l; i++) {
        if (text[i] == '\\') {
            if (text[i+1] == 'n') {
                parsed[index++] = '\n';
                i++;
                continue;
            } else if (text[i+1] == 't') {
                parsed[index++] = '\t';
                i++;
                continue;
            }

            parsed[index++] = '\\';
        } else {
            parsed[index++] = text[i];
        }
    }

    parsed[index] = '\0';

    *output = parsed;

    return rc;
}

unsigned int get_json(char **value, char *key, char *url) {
    unsigned int rc = RETURN_OK, http_code = 0, text_length = 0, token_count = 0;
    unsigned int i = 0, l = 0, string_length = 0;
    char *text = NULL, *string_value = NULL;
    struct json_token *tokens = NULL;

    switch ((http_code = get_request(&text, &text_length, url, MIMETYPE_JSON))) {
    case 503:
        error_program_log("Could not reach the registry");

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log("Library does not exist");

        rc = RETURN_EPERM;
        goto cleanup;
    case 200:
    default:
        break;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    token_count = parse_json(&tokens, text, 22);

    if (token_count == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!tokens) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = token_count; i < l; i++) {
        if (tokens[i].key == TRUE) {
            if (strncmp(text+tokens[i].start, key, strlen(key)) == 0) {
                string_length = tokens[i+1].end - tokens[i+1].start;
                string_value = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

                strncpy(string_value, text+tokens[i+1].start, (size_t)string_length);
                string_value[string_length] = '\0';

                break;
            }
        }
    }

    if (!string_value) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (string_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = parse_text(&*value, string_value)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(text);
    nessemble_free(tokens);

    return rc;
}

unsigned int get_json_search(char *url, char *term) {
    unsigned int rc = RETURN_OK, text_length = 0;
    unsigned int i = 0, k = 0, l = 0;
    unsigned int token_count = 0, string_length = 0, results_index = 0;
    char *text = NULL;
    char *results[200];
    struct json_token *tokens = NULL;

    switch (get_request(&text, &text_length, url, MIMETYPE_JSON)) {
    case 503:
        error_program_log("Could not reach the registry");

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log("Library does not exist");

        rc = RETURN_EPERM;
        goto cleanup;
    case 200:
    default:
        break;
    }

    if (!text) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    token_count = parse_json(&tokens, text, 100);

    if (token_count == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!tokens) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = token_count; i < l; i++) {
        if (tokens[i].key == TRUE) {
            string_length = tokens[i].end - tokens[i].start;

            if (string_length == 4 && strncmp(text+tokens[i].start, "name", 4) == 0) {
                string_length = tokens[i+1].end - tokens[i+1].start;
                results[results_index] = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

                memset(results[results_index], 0, (size_t)string_length);
                strncpy(results[results_index], text+tokens[i+1].start, (size_t)string_length);
                results[results_index][string_length] = '\0';

                results_index++;
            }

            string_length = tokens[i].end - tokens[i].start;

            if (string_length == 11 && strncmp(text+tokens[i].start, "description", 11) == 0) {
                string_length = tokens[i+1].end - tokens[i+1].start;
                results[results_index] = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

                memset(results[results_index], 0, (size_t)string_length);
                strncpy(results[results_index], text+tokens[i+1].start, (size_t)string_length);
                results[results_index][string_length] = '\0';

                results_index++;
            }
        }
    }

    i = 0;

    while (i < results_index) {
        int name_index = 0, description_index = 0, term_length = 0;
        char *name_text = NULL, *description_text = NULL;

        name_text = results[i++];
        description_text = results[i++];

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
    nessemble_free(text);
    nessemble_free(tokens);

    for (i = 0, l = results_index; i < l; i++) {
        nessemble_free(results[i]);
    }

    return rc;
}
