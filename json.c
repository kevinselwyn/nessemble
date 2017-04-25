#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "third-party/jsmn/jsmn.h"

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}

	return -1;
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

unsigned int get_json_buffer(char **value, char *key, char *json) {
    int token_count = 0;
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, l = 0, string_length = 0;
    char *string_value = NULL;
    jsmn_parser parser;
    jsmntok_t tokens[JSON_TOKEN_MAX];

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, json, strlen(json), tokens, JSON_TOKEN_MAX);

    if (token_count <= 0 || tokens[0].type != JSMN_OBJECT) {
        error_program_log(_("Could not parse JSON"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 1, l = (unsigned int)token_count; i < l; i++) {
        if (jsoneq(json, &tokens[i], key) == 0) {
            string_length = tokens[i+1].end - tokens[i+1].start;
            string_value = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

            strncpy(string_value, json+tokens[i+1].start, (size_t)string_length);
            string_value[string_length] = '\0';

            break;
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
    return rc;
}

unsigned int get_json_file(char **value, char *key, char *filename) {
    unsigned int rc = RETURN_OK;
    unsigned int file_length = 0;
    char *file_data = NULL;

    if ((rc = load_file(&file_data, &file_length, filename)) != RETURN_OK) {
        goto cleanup;
    }

    rc = get_json_buffer(&*value, key, file_data);

cleanup:
    nessemble_free(file_data);

    return rc;
}

unsigned int get_json_url(char **value, char *key, char *url) {
    unsigned int rc = RETURN_OK, text_length = 0;
    char *text = NULL;
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } } };

    /* options */
    download_options.response = &text;
    download_options.response_length = &text_length;
    download_options.url = url;
    download_options.data_length = 1024 * 512;
    download_options.mime_type = MIMETYPE_JSON;

    switch (get_request(download_options)) {
    case 503:
        error_program_log(_("Could not reach the registry"));

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log(_("Library does not exist"));

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

    if ((rc = get_json_buffer(&*value, key, text)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(text);

    return rc;
}

unsigned int get_json_search(char *url, char *term) {
    int token_count = 0;
    unsigned int rc = RETURN_OK, text_length = 0;
    unsigned int i = 0, k = 0, l = 0;
    unsigned int string_length = 0, results_index = 0;
    char *text = NULL;
    char *results[200];
    jsmn_parser parser;
    jsmntok_t tokens[JSON_TOKEN_MAX];
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } } };

    /* options */
    download_options.response = &text;
    download_options.response_length = &text_length;
    download_options.url = url;
    download_options.data_length = 1024 * 512;
    download_options.mime_type = MIMETYPE_JSON;

    switch (get_request(download_options)) {
    case 503:
        error_program_log(_("Could not reach the registry"));

        rc = RETURN_EPERM;
        goto cleanup;
    case 404:
        error_program_log(_("Library does not exist"));

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

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, text, strlen(text), tokens, JSON_TOKEN_MAX);

    if (token_count <= 0 || tokens[0].type != JSMN_OBJECT) {
        error_program_log(_("Could not parse JSON"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 1, l = token_count; i < l; i++) {
        if (jsoneq(text, &tokens[i], "name") == 0) {
            string_length = tokens[i+1].end - tokens[i+1].start;
            results[results_index] = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

            memset(results[results_index], 0, (size_t)string_length);
            strncpy(results[results_index], text+tokens[i+1].start, (size_t)string_length);
            results[results_index][string_length] = '\0';

            results_index++;
        }

        if (jsoneq(text, &tokens[i], "description") == 0) {
            string_length = tokens[i+1].end - tokens[i+1].start;
            results[results_index] = (char *)nessemble_malloc(sizeof(char) * (string_length + 1));

            memset(results[results_index], 0, (size_t)string_length);
            strncpy(results[results_index], text+tokens[i+1].start, (size_t)string_length);
            results[results_index][string_length] = '\0';

            results_index++;
        }
    }

    i = 0;

    while (i < results_index) {
        int name_index = 0, description_index = 0, term_length = 0;
        char *name_text = NULL, *description_text = NULL;

        name_text = results[i++];
        description_text = results[i++];

        term_length = (int)strlen(term);
        name_index = nessemble_strcasestr(name_text, term) - name_text;
        description_index = nessemble_strcasestr(description_text, term) - description_text;

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

    for (i = 0, l = results_index; i < l; i++) {
        nessemble_free(results[i]);
    }

    return rc;
}
