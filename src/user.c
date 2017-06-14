#include <string.h>
#include <unistd.h>
#include "nessemble.h"
#include "http.h"

unsigned int user_auth(http_t *request, char *token, char *method, char *route) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *username = NULL, *token_hash = NULL, *base64 = NULL;
    char *credentials = NULL, *authorization = NULL, *data = NULL;
    http_t local_request;

    local_request = *request;

    if ((rc = get_config(&username, "username")) != RETURN_OK) {
        error_program_log(_("User not logged in"));
        goto cleanup;
    }

    length = strlen(method) + strlen(route) + 1;
    data = (char *)nessemble_malloc(sizeof(char) * (length + 1));
    sprintf(data, "%s+%s", method, route);

    hmac(&token_hash, token, strlen(token), data, strlen(data));

    length = strlen(username) + strlen(token_hash) + 1;
    credentials = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    sprintf(credentials, "%s:%s", username, token_hash);

    if ((rc = base64enc(&base64, credentials)) != RETURN_OK) {
        goto cleanup;
    }

    length = strlen(base64) + 10;
    authorization = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    sprintf(authorization, "HMAC-SHA1 %s", base64);

    if ((rc = http_header(&local_request, "Authorization", authorization)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    *request = local_request;

    nessemble_free(username);
    nessemble_free(token_hash);
    nessemble_free(credentials);
    nessemble_free(base64);
    nessemble_free(authorization);
    nessemble_free(data);

    return rc;
}

unsigned int user_create() {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_name = NULL, *user_username = NULL, *user_email = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

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

    while (get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
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

    sprintf(data, "{\n\t\"name\":\"%s\",\n\t\"username\":\"%s\",\n\t\"email\":\"%s\",\n\t\"password\":\"%s\"\n}", user_name, user_username, user_email, user_password);

    if ((rc = api_user(&url, "create")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = http_header(&request, "Accept", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Content-Type", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_data(&request, data, strlen(data))) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_post(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (http_header_cmp(request, "Content-Type", MIMETYPE_JSON) != 0) {
        error_program_log(_("Invalid type"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    http_release(&request);

cleanup:
    nessemble_free(user_name);
    nessemble_free(user_username);
    nessemble_free(user_email);

    return rc;
}

unsigned int user_login() {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL, *token = NULL;
    char *base64 = NULL, *auth = NULL;
    char *user_username = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);
    auth = (char *)nessemble_malloc(sizeof(char) * 1024);

    while (get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    sprintf(data, "%s:%s", user_username, user_password);

    if ((rc = base64enc(&base64, data)) != RETURN_OK) {
        goto cleanup;
    }

    sprintf(auth, "Basic %s", base64);

    if ((rc = api_user(&url, "login")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = http_header(&request, "Accept", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Content-Type", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Authorization", auth)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_post(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&token, "token", request.response_body)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = set_config(user_username, "username")) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = set_config(token, "login")) != RETURN_OK) {
        goto cleanup;
    }

    http_release(&request);

cleanup:
    nessemble_free(url);
    nessemble_free(buffer);
    nessemble_free(user_username);
    nessemble_free(auth);
    nessemble_free(token);

    return rc;
}

unsigned int user_logout() {
    unsigned int rc = RETURN_OK;
    char *url = NULL, *error = NULL, *token = NULL;
    http_t request;

    if ((rc = get_config(&token, "login")) != RETURN_OK) {
        error_program_log(_("User not logged in"));
        goto cleanup;
    }

    if ((rc = api_user(&url, "logout")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = user_auth(&request, token, "POST", "/user/logout")) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Accept", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Content-Type", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_data(&request, "{}", 2)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_post(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
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

    http_release(&request);

cleanup:
    nessemble_free(token);
    nessemble_free(url);
    nessemble_free(error);

    return rc;
}

unsigned int user_forgotpassword() {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_username = NULL;
    char *json_email = NULL, *json_url = NULL;
    char data[1024];
    http_t request;

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    while (get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    sprintf(data, "{\n\t\"username\":\"%s\"\n}", user_username);

    if ((rc = api_user(&url, "forgotpassword")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = http_header(&request, "Accept", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Content-Type", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_data(&request, data, strlen(data))) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_post(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&json_email, "email", request.response_body)) != RETURN_OK) {
        goto cleanup;
    }

    if (strcmp(json_email, "false") == 0) {
        if ((rc = get_json_buffer(&json_url, "url", request.response_body)) != RETURN_OK) {
            goto cleanup;
        }

        printf("%s\n\n%s:\n  %s\n", _("Could not send password reset email"), _("Authenticate with this QR code"), json_url);
    } else {
        printf("%s\n", _("Password reset email sent"));
    }

    http_release(&request);

cleanup:
    nessemble_free(url);
    nessemble_free(buffer);
    nessemble_free(user_username);
    nessemble_free(json_email);
    nessemble_free(json_url);

    return rc;
}

unsigned int user_resetpassword() {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_token = NULL, *user_username = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

    memset(data, '\0', 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    while (get_line(&buffer, _("Token: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_token = nessemble_strdup(buffer);
        break;
    }

    while (get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    if ((rc = set_config(user_username, "username")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = user_auth(&request, user_token, "POST", "/user/resetpassword")) != RETURN_OK) {
        goto cleanup;
    }

    sprintf(data, "{\n\t\"username\":\"%s\",\n\t\"password\":\"%s\"\n}", user_username, user_password);

    if ((rc = api_user(&url, "resetpassword")) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Accept", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_header(&request, "Content-Type", MIMETYPE_JSON)) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_data(&request, data, strlen(data))) != RETURN_OK) {
        goto cleanup;
    }

    if ((rc = http_post(&request, url)) != RETURN_OK) {
        error_program_log(_("Could not reach the registry"));
        goto cleanup;
    }

    if (!request.content_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            error_program_log(error);
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    http_release(&request);

cleanup:
    nessemble_free(url);
    nessemble_free(buffer);
    nessemble_free(user_token);
    nessemble_free(user_username);

    return rc;
}
