#include <string.h>
#include <unistd.h>
#include "nessemble.h"

static unsigned int user_url(char **url, char *endpoint) {
    unsigned int rc = RETURN_OK;
    size_t length_registry = 0, length_endpoint = 0;
    char *registry = NULL, *path = NULL;

    if ((rc = get_registry(&registry)) != RETURN_OK) {
        goto cleanup;
    }

    length_registry = strlen(registry);
    length_endpoint = strlen(endpoint);
    path = (char *)nessemble_malloc(sizeof(char) * (length_registry + length_endpoint + 6) + 1);

    sprintf(path, "%s/user/%s", registry, endpoint);

    *url = path;

cleanup:
    nessemble_free(registry);

    return rc;
}

unsigned int user_create() {
    unsigned int rc = RETURN_OK;
    unsigned int http_code = 0, response_length = 0;
    size_t length = 0;
    char *url = NULL, *response = NULL, *error = NULL, *buffer = NULL;
    char *user_name = NULL, *user_email = NULL, *user_password = NULL;
    char data[1024];
    struct http_header http_headers = { 0, {}, {} };

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    while (get_line(&buffer, "Name: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_name = nessemble_strdup(buffer);
        break;
    }

    while (get_line(&buffer, "Email: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_email = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = getpass("Password: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    sprintf(data, "{\n\t\"name\":\"%s\",\n\t\"email\":\"%s\",\n\t\"password\":\"%s\"\n}", user_name, user_email, user_password);

    if ((rc = user_url(&url, "create")) != RETURN_OK) {
        goto cleanup;
    }

    http_code = post_request(&response, &response_length, url, data, MIMETYPE_JSON, http_headers);

    if (!response || response_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_code != 200) {
        if ((rc = get_json_value(&error, "error", response)) != RETURN_OK) {
            error_program_log("Could not read response");
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    nessemble_free(url);
    nessemble_free(response);
    nessemble_free(buffer);
    nessemble_free(user_name);
    nessemble_free(user_email);

    return rc;
}

unsigned int user_login() {
    unsigned int rc = RETURN_OK;
    unsigned int http_code = 0, response_length = 0;
    size_t length = 0;
    char *url = NULL, *response = NULL, *error = NULL, *buffer = NULL, *token = NULL;
    char *base64 = NULL, *auth = NULL;
    char *user_email = NULL, *user_password = NULL;
    char data[1024];
    struct http_header http_headers = { 0, {}, {} };

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);
    auth = (char *)nessemble_malloc(sizeof(char) * 1024);

    while (get_line(&buffer, "Email: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_email = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = getpass("Password: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    sprintf(data, "%s:%s", user_email, user_password);

    if ((rc = base64enc(&base64, data)) != RETURN_OK) {
        goto cleanup;
    }

    sprintf(auth, "Basic %s", base64);

    http_headers.keys[http_headers.count] = "Authorization";
    http_headers.vals[http_headers.count++] = nessemble_strdup(auth);

    if ((rc = user_url(&url, "login")) != RETURN_OK) {
        goto cleanup;
    }

    http_code = post_request(&response, &response_length, url, data, MIMETYPE_JSON, http_headers);

    if (!response || response_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_code != 200) {
        if ((rc = get_json_value(&error, "error", response)) != RETURN_OK) {
            error_program_log("Could not read response");
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_value(&token, "token", response)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = set_config(token, "login")) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(url);
    nessemble_free(response);
    nessemble_free(buffer);
    nessemble_free(user_email);
    nessemble_free(auth);
    nessemble_free(token);

    return rc;
}
