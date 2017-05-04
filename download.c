#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "nessemble.h"

#ifdef IS_WINDOWS
#include <winsock2.h>
#else /* IS_WINDOWS */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* IS_WINDOWS */

static unsigned int do_request(struct download_option download_options) {
    unsigned int port = 80, protocol = PROTOCOL_HTTP;
    unsigned int i = 0, l = 0, index = 0;
    unsigned int code = 200, length = 0;
    int sockfd = 0, bytes = 0, sent = 0, received = 0, total = 0;
    int content_type_index = 0, response_index = 0, content_length_index = 0;
    size_t message_length = 0;
    char message[2048], response[4096], code_str[4];
    char *host = NULL, *uri = NULL;
    char *output = NULL;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    fd_set set;

    /* options */
    char **request = download_options.response;
    unsigned int *request_length = download_options.response_length;
    char *url = download_options.url;
    char *data = download_options.data;
    unsigned int data_length = download_options.data_length;
    char *method = download_options.method;
    char *mime_type = download_options.mime_type;
    struct http_header http_headers = download_options.http_headers;

    if (strncmp(url, "http:", 5) == 0) {
        protocol = PROTOCOL_HTTP;
    } else if (strncmp(url, "https:", 6) == 0) {
        protocol = PROTOCOL_HTTPS;
    } else {
        error_program_log(_("Protocol is not supported"));
        code = 500;
        goto cleanup;
    }

    if (protocol == PROTOCOL_HTTPS) {
        error_program_log(_("HTTPS is not supported"));
        code = 500;
    }

    host = nessemble_strdup(url+(protocol == PROTOCOL_HTTP ? 7 : 8));

    for (i = 0, l = (unsigned int)strlen(host); i < l; i++) {
        if (host[i] == '/') {
            index = i;
            break;
        }
    }

    if (index == 0) {
        uri = nessemble_strdup("");
    } else {
        uri = nessemble_strdup(host+index);
    }

    host[index] = '\0';

    index = 0;

    for (i = 0, l = (unsigned int)strlen(host); i < l; i++) {
        if (host[i] == ':') {
            index = i;
            break;
        }
    }

    if (index == 0) {
        port = 80;
    } else {
        port = (unsigned int)atoi(host+(index+1));
        host[index] = '\0';
    }

    message_length += sprintf(message, "%s %s HTTP/1.1\r\nHost: %s", method, uri, host);

    if (port != 80) {
        message_length += sprintf(message+message_length, ":%u", port);
    }

    message_length += sprintf(message+message_length, "\r\nUser-Agent: " PROGRAM_NAME "/" PROGRAM_VERSION);
    message_length += sprintf(message+message_length, "\r\nConnection: keep-alive");
    message_length += sprintf(message+message_length, "\r\nCache-Control: no-cache");
    message_length += sprintf(message+message_length, "\r\nAccept: */*");
    message_length += sprintf(message+message_length, "\r\nAccept-Language: " PROGRAM_LANGUAGE ";q=0.8");
    message_length += sprintf(message+message_length, "\r\nContent-Type: %s", mime_type);

    for (i = 0, l = http_headers.count; i < l; i++) {
        message_length += sprintf(message+message_length, "\r\n%s: %s", http_headers.keys[i], http_headers.vals[i]);
    }

    if (data) {
        message_length += sprintf(message+message_length, "\r\nContent-Length: %u\r\n\r\n", data_length);

        memcpy(message+message_length, data, data_length);
        message_length += data_length;
    } else {
        message_length += sprintf(message+message_length, "\r\n\r\n");
    }

#ifdef IS_WINDOWS
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error_program_log(_("Could not initialize %s"), "Winsock");
        code = 500;
        goto cleanup;
    }
#endif /* IS_WINDOWS */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_program_log(_("Could not open socket"));
        code = 500;
        goto cleanup;
    }

    server = gethostbyname(host);

    if (!server) {
        error_program_log(_("Could not find host `%s`"), host);
        code = 404;
        goto cleanup;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error_program_log(_("Could not connect to host `%s`"), host);
        code = 500;
        goto cleanup;
    }

    total = (int)message_length;
    sent = 0;

    do {
#ifdef IS_WINDOWS
        bytes = (int)send(sockfd, message+sent, (size_t)(total - sent), 0);
#else /* IS_WINDOWS */
        bytes = (int)write(sockfd, message+sent, (size_t)(total - sent));
#endif /* IS_WINDOWS */

        if (bytes < 0) {
            error_program_log(_("Problem sending request"));
            code = 500;
            goto cleanup;
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);

    total = 4096;
    received = 0;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    do {
        if (select(sockfd+1, &set, NULL, NULL, &timeout) <= 0) {
            break;
        }

#ifdef IS_WINDOWS
        bytes = (int)recv(sockfd, response+received, (size_t)(total - received), 0);
#else /* IS_WINDOWS */
        bytes = (int)read(sockfd, response+received, (size_t)(total - received));
#endif /* IS_WINDOWS */

        if (bytes < 0) {
            error_program_log(_("Problem receiving response"));
            code = 500;
            goto cleanup;
        }

        if (bytes == 0) {
            break;
        }

        received += bytes;
    } while (received < total);

    if (received == total) {
        error_program_log(_("Problem receiving response"));
        code = 500;
        goto cleanup;
    }

    UNUSED(close(sockfd));

    strncpy(code_str, response+9, 3);
    code_str[3] = '\0';

    code = (unsigned int)atoi(code_str);

    if (code != 200) {
        error_program_log(_("HTTP code `%u` returned"), code);
    }

    content_type_index = nessemble_strcasestr(response, "Content-Type") - response;

    if (content_type_index == 0) {
        error_program_log(_("Could not read `Content-Type`"));
        code = 500;
        goto cleanup;
    }

    content_type_index += 14;

    if (strncmp(response+content_type_index, mime_type, strlen(mime_type)) != 0) {
        error_program_log(_("Incorrect Content-Type `%s`"), mime_type);
        code = 500;
        goto cleanup;
    }

    content_length_index = nessemble_strcasestr(response, "Content-Length") - response;

    if (content_length_index == 0) {
        error_program_log(_("Invalid `Content-Length`"));
        code = 500;
        goto cleanup;
    }

    length = (unsigned int)atoi(response+(content_length_index+16));

    response_index = strstr(response, "\r\n\r\n") - response;

    if (response_index == 0) {
        error_program_log(_("No response found"));
        code = 500;
        goto cleanup;
    }

    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    memcpy(output, response+(response_index+4), (size_t)length);
    output[length] = '\0';

cleanup:
    *request = output;
    *request_length = length;

    nessemble_free(host);
    nessemble_free(uri);

    for (i = 0, l = http_headers.count; i < l; i++) {
        nessemble_free(http_headers.vals[i]);
    }

    return code;
}

unsigned int get_request(struct download_option download_options) {
    struct http_header http_headers = { 0, {}, {} };

    if (!download_options.method) {
        download_options.method = "GET";
    }

    download_options.http_headers = http_headers;

    return do_request(download_options);
}

unsigned int post_request(struct download_option download_options) {
    if (!download_options.method) {
        download_options.method = "POST";
    }

    return do_request(download_options);
}
