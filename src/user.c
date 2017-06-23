#include <string.h>
#include <unistd.h>
#include "nessemble.h"
#include "http.h"

unsigned int user_auth(http_t *request, char *token, char *method, char *route) {
    unsigned int rc = RETURN_OK;
    size_t data_length = 0, credentials_length = 0, authorization_length = 0;
    char *username = NULL, *token_hash = NULL, *base64 = NULL;
    char *credentials = NULL, *authorization = NULL, *data = NULL;
    http_t local_request;

    local_request = *request;

    if ((rc = get_config(&username, "username")) != RETURN_OK) {
        error_program_log(_("User not logged in"));
        goto cleanup;
    }

    if (!username) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data_length = strlen(method) + strlen(route) + 1;
    data = (char *)nessemble_malloc(sizeof(char) * (data_length + 1));

    if (snprintf(data, data_length+1, "%s+%s", method, route) != (int)data_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    hmac(&token_hash, token, strlen(token), data, strlen(data));

    if (!token_hash) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    credentials_length = strlen(username) + strlen(token_hash) + 1;
    credentials = (char *)nessemble_malloc(sizeof(char) * (credentials_length + 1));

    if (snprintf(credentials, credentials_length+1, "%s:%s", username, token_hash) != (int)credentials_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = base64enc(&base64, credentials)) != RETURN_OK) {
        goto cleanup;
    }

    if (!base64) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    authorization_length = strlen(base64) + 10;
    authorization = (char *)nessemble_malloc(sizeof(char) * (authorization_length + 1));

    if (snprintf(authorization, authorization_length+1, "HMAC-SHA1 %s", base64) != (int)authorization_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

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

unsigned int user_create(int optc, char *optv[]) {
    unsigned int rc = RETURN_OK;
    size_t length = 0, data_length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_name = NULL, *user_username = NULL, *user_email = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

    memset(data, 0, 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    if (optc >= 1) {
        user_name = nessemble_strdup(optv[0]);
    }

    while (!user_name && get_line(&buffer, _("Name: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_name = nessemble_strdup(buffer);
        break;
    }

    if (!user_name) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (optc >= 2) {
        user_username = nessemble_strdup(optv[1]);
    }

    while (!user_username && get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    if (!user_username) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (optc >= 3) {
        user_email = nessemble_strdup(optv[2]);
    }

    while (!user_email && get_line(&buffer, _("Email: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_email = nessemble_strdup(buffer);
        break;
    }

    if (!user_email) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    if (!user_password) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data_length = strlen(user_name) + strlen(user_username) + strlen(user_email) + strlen(user_password) + 59;

    if (snprintf(data, data_length+1, "{\n\t\"name\":\"%s\",\n\t\"username\":\"%s\",\n\t\"email\":\"%s\",\n\t\"password\":\"%s\"\n}", user_name, user_username, user_email, user_password) != (int)data_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = api_user(&url, "create")) != RETURN_OK) {
        goto cleanup;
    }

    if (!url) {
        rc = RETURN_EPERM;
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

    if (request.status_code != 201) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            if (error) {
                error_program_log(error);
            }
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.content_length == 0) {
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

unsigned int user_login(int optc, char *optv[]) {
    unsigned int rc = RETURN_OK;
    size_t length = 0, data_length = 0, auth_length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL, *token = NULL;
    char *base64 = NULL, *auth = NULL;
    char *user_username = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

    memset(data, 0, 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);
    auth = (char *)nessemble_malloc(sizeof(char) * 1024);

    if (optc >= 1) {
        user_username = nessemble_strdup(optv[0]);
    }

    while (!user_username && get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    if (!user_username) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    if (!user_password) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data_length = strlen(user_username) + strlen(user_password) + 1;

    if (snprintf(data, data_length+1, "%s:%s", user_username, user_password) != (int)data_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = base64enc(&base64, data)) != RETURN_OK) {
        goto cleanup;
    }

    if (!base64) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    auth_length = strlen(base64) + 6;

    if (snprintf(auth, auth_length+1, "Basic %s", base64) != (int)auth_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = api_user(&url, "login")) != RETURN_OK) {
        goto cleanup;
    }

    if (!url) {
        rc = RETURN_EPERM;
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

    if (request.content_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            if (error) {
                error_program_log(error);
            }
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&token, "token", request.response_body)) != RETURN_OK) {
        goto cleanup;
    }

    if (!token) {
        rc = RETURN_EPERM;
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

    if (!token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = api_user(&url, "logout")) != RETURN_OK) {
        goto cleanup;
    }

    if (!url) {
        rc = RETURN_EPERM;
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

    if (request.content_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            if (error) {
                error_program_log(error);
            }
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

unsigned int user_forgotpassword(int optc, char *optv[]) {
    unsigned int rc = RETURN_OK;
    size_t length = 0, data_length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_username = NULL;
    char *json_email = NULL, *json_url = NULL;
    char data[1024];
    http_t request;

    memset(data, 0, 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    if (optc >= 1) {
        user_username = nessemble_strdup(optv[0]);
    }

    while (!user_username && get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    if (!user_username) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data_length = strlen(user_username) + 18;

    if (snprintf(data, data_length+1, "{\n\t\"username\":\"%s\"\n}", user_username) != (int)data_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = api_user(&url, "forgotpassword")) != RETURN_OK) {
        goto cleanup;
    }

    if (!url) {
        rc = RETURN_EPERM;
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

    if (request.content_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            if (error) {
                error_program_log(error);
            }
        }

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = get_json_buffer(&json_email, "email", request.response_body)) != RETURN_OK) {
        goto cleanup;
    }

    if (!json_email) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (strcmp(json_email, "false") == 0) {
        if ((rc = get_json_buffer(&json_url, "url", request.response_body)) != RETURN_OK) {
            goto cleanup;
        }

        if (!json_url) {
            rc = RETURN_EPERM;
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

unsigned int user_resetpassword(int optc, char *optv[]) {
    unsigned int rc = RETURN_OK;
    size_t length = 0, data_length = 0;
    char *url = NULL, *error = NULL, *buffer = NULL;
    char *user_token = NULL, *user_username = NULL, *user_password = NULL;
    char data[1024];
    http_t request;

    memset(data, 0, 1024);
    buffer = (char *)nessemble_malloc(sizeof(char) * BUF_GET_LINE);

    if (optc >= 1) {
        user_token = nessemble_strdup(optv[0]);
    }

    while (!user_token && get_line(&buffer, _("Token: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_token = nessemble_strdup(buffer);
        break;
    }

    if (!user_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (optc >= 2) {
        user_username = nessemble_strdup(optv[1]);
    }

    while (!user_username && get_line(&buffer, _("Username: ")) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        user_username = nessemble_strdup(buffer);
        break;
    }

    if (!user_username) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while ((user_password = nessemble_getpass(_("Password: "))) != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        break;
    }

    if (!user_password) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = set_config(user_username, "username")) != RETURN_OK) {
        goto cleanup;
    }

    http_init(&request);

    if ((rc = user_auth(&request, user_token, "POST", "/user/resetpassword")) != RETURN_OK) {
        goto cleanup;
    }

    data_length = strlen(user_username) + strlen(user_password) + 34;

    if (snprintf(data, data_length+1, "{\n\t\"username\":\"%s\",\n\t\"password\":\"%s\"\n}", user_username, user_password) != (int)data_length) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = api_user(&url, "resetpassword")) != RETURN_OK) {
        goto cleanup;
    }

    if (!url) {
        rc = RETURN_EPERM;
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

    if (request.content_length == 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (request.status_code != 200) {
        if ((rc = get_json_buffer(&error, "error", request.response_body)) != RETURN_OK) {
            error_program_log(_("Could not read response"));
        } else {
            if (error) {
                error_program_log(error);
            }
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
