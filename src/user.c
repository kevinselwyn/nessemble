#include <string.h>
#include <unistd.h>
#include "nessemble.h"

unsigned int user_auth(struct http_header *http_headers) {
    unsigned int rc = RETURN_OK;
    char *token = NULL;

    if ((rc = get_config(&token, "login")) != RETURN_OK) {
        error_program_log(_("User not logged in"));
        goto cleanup;
    }

    http_headers->keys[http_headers->count] = "X-Auth-Token";
    http_headers->vals[http_headers->count++] = nessemble_strdup(token);

cleanup:
    nessemble_free(token);

    return rc;
}

unsigned int user_create() {
    unsigned int rc = RETURN_OK;
    unsigned int http_code = 0, response_length = 0;
    size_t length = 0;
    char *url = NULL, *response = NULL, *error = NULL, *buffer = NULL;
    char *user_name = NULL, *user_email = NULL, *user_password = NULL;
    char data[1024];
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header http_headers = { 0, {}, {} };
    struct http_header response_headers = { 0, {}, {} };

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    while (get_line(&buffer, _("Name: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_name = nessemble_strdup(buffer);
        break;
    }

    while (get_line(&buffer, _("Email: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_email = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    sprintf(data, "{\n\t\"name\":\"%s\",\n\t\"email\":\"%s\",\n\t\"password\":\"%s\"\n}", user_name, user_email, user_password);

    if ((rc = api_user(&url, "create")) != RETURN_OK) {
        goto cleanup;
    }

    /* options */
    download_options.response = &response;
    download_options.response_length = &response_length;
    download_options.url = url;
    download_options.data = data;
    download_options.data_length = strlen(data);
    download_options.mime_type = MIMETYPE_JSON;
    download_options.http_headers = http_headers;
    download_options.response_headers = &response_headers;

    http_code = post_request(download_options);

    if (!response || response_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_code != 200) {
        if ((rc = get_json_buffer(&error, "error", response)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
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
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header http_headers = { 0, {}, {} };
    struct http_header response_headers = { 0, {}, {} };

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);
    auth = (char *)nessemble_malloc(sizeof(char) * 1024);

    while (get_line(&buffer, _("Email: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_email = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
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

    if ((rc = api_user(&url, "login")) != RETURN_OK) {
        goto cleanup;
    }

    /* options */
    download_options.response = &response;
    download_options.response_length = &response_length;
    download_options.url = url;
    download_options.data = NULL;
    download_options.data_length = 0;
    download_options.mime_type = MIMETYPE_JSON;
    download_options.http_headers = http_headers;
    download_options.response_headers = &response_headers;

    http_code = post_request(download_options);

    if (!response || response_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_code != 200) {
        if ((rc = get_json_buffer(&error, "error", response)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&token, "token", response)) != RETURN_OK) {
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

unsigned int user_logout() {
    unsigned int rc = RETURN_OK;
    unsigned int http_code = 0, response_length = 0;
    char *url = NULL, *response = NULL, *error = NULL;
    struct download_option download_options = { 0, 0, NULL, NULL, NULL, NULL, NULL, { 0, { }, { } }, NULL };
    struct http_header http_headers = { 0, {}, {} };
    struct http_header response_headers = { 0, {}, {} };

    if ((rc = user_auth(&http_headers)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = api_user(&url, "logout")) != RETURN_OK) {
        goto cleanup;
    }

    /* options */
    download_options.response = &response;
    download_options.response_length = &response_length;
    download_options.url = url;
    download_options.data = "{}";
    download_options.data_length = 1024;
    download_options.mime_type = MIMETYPE_JSON;
    download_options.http_headers = http_headers;
    download_options.response_headers = &response_headers;

    http_code = post_request(download_options);

    if (!response || response_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_code != 200) {
        if ((rc = get_json_buffer(&error, "error", response)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = set_config(NULL, "login")) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    return rc;
}
