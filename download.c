#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "nessemble.h"
#include "download.h"

#include <string.h>
char *strstr(const char *haystack, const char *needle);
#define _GNU_SOURCE
#include <string.h>
char *strcasestr(const char *haystack, const char *needle);

unsigned int get_request(char **request, unsigned int *request_length, char *url, char *mime_type) {
    unsigned int port = 80, protocol = PROTOCOL_HTTP;
    unsigned int i = 0, l = 0, index = 0;
    unsigned int code = 0, length = 0;
    int sockfd = 0, bytes = 0, sent = 0, received = 0, total = 0;
    int content_type_index = 0, response_index = 0, content_length_index = 0;
    char message[1024], response[4096], code_str[4];
    char *host = NULL, *uri = NULL;
    char *data = NULL;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    fd_set set;

    if (strncmp(url, "http:", 5) == 0) {
        protocol = PROTOCOL_HTTP;
    } else if (strncmp(url, "https:", 6) == 0) {
        protocol = PROTOCOL_HTTPS;
    } else {
        error_program_log("Protocol is not supported");
        code = 500;
        goto cleanup;
    }

    if (protocol == PROTOCOL_HTTPS) {
        error_program_log("HTTPS is not supported");
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

    sprintf(message, "GET %s HTTP/1.1\r\nHost: %s", uri, host);

    if (port != 80) {
        sprintf(message+strlen(message), ":%u", port);
    }

    sprintf(message+strlen(message), "\r\nUser-Agent: " PROGRAM_NAME "/" PROGRAM_VERSION);
    sprintf(message+strlen(message), "\r\nConnection: keep-alive");
    sprintf(message+strlen(message), "\r\nCache-Control: no-cache");
    sprintf(message+strlen(message), "\r\nAccept: */*");
    sprintf(message+strlen(message), "\r\nAccept-Language: " PROGRAM_LANGUAGE ";q=0.8");
    sprintf(message+strlen(message), "\r\nContent-Type: %s\r\n\r\n", mime_type);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_program_log("Could not open socket");
        code = 500;
        goto cleanup;
    }

    server = gethostbyname(host);

    if (!server) {
        error_program_log("Could not find host `%s`", host);
        code = 404;
        goto cleanup;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, (socklen_t)sizeof(serv_addr)) < 0) {
        error_program_log("Could not connect to host `%s`", host);
        code = 500;
        goto cleanup;
    }

    total = (int)strlen(message);
    sent = 0;

    do {
        bytes = (int)write(sockfd, message+sent, (size_t)(total - sent));

        if (bytes < 0) {
            code = 500;
            goto cleanup;
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);

    memset(response, 0, sizeof(response));
    total = (int)sizeof(response) - 1;
    received = 0;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    do {
        if (select(sockfd+1, &set, NULL, NULL, &timeout) <= 0) {
            break;
        }

        bytes = (int)read(sockfd, response+received, (size_t)(total - received));

        if (bytes < 0) {
            code = 500;
            goto cleanup;
        }

        if (bytes == 0) {
            break;
        }

        received += bytes;
    } while (received < total);

    if (received == total) {
        code = 500;
        goto cleanup;
    }

    (void)close(sockfd);

    strncpy(code_str, response+9, 3);
    code_str[3] = '\0';

    code = (unsigned int)atoi(code_str);

    if (code != 200) {
        error_program_log("HTTP code `%u` returned", code);
        goto cleanup;
    }

    content_type_index = strcasestr(response, "Content-Type") - response;

    if (content_type_index == 0) {
        error_program_log("Could not read `Content-Type`");
        code = 500;
        goto cleanup;
    }

    content_type_index += 14;

    if (strncmp(response+content_type_index, mime_type, strlen(mime_type)) != 0) {
        error_program_log("Incorrect Content-Type `%s`", mime_type);
        code = 500;
        goto cleanup;
    }

    content_length_index = strcasestr(response, "Content-Length") - response;

    if (content_length_index == 0) {
        error_program_log("Invalid `Content-Length`");
        code = 500;
        goto cleanup;
    }

    length = (unsigned int)atoi(response+(content_length_index+16));

    response_index = strstr(response, "\r\n\r\n") - response;

    if (response_index == 0) {
        error_program_log("No response found");
        code = 500;
        goto cleanup;
    }

    data = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    memcpy(data, response+(response_index+4), (size_t)length);
    data[length] = '\0';

cleanup:
    *request = data;
    *request_length = length;

    nessemble_free(host);
    nessemble_free(uri);

    return code;
}
