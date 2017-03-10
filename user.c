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

    http_code = post_request(&response, &response_length, url, data, MIMETYPE_JSON);

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
    nessemble_free(user_name);
    nessemble_free(user_email);

    return rc;
}
