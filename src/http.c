#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "http.h"
#include "nessemble.h"

#ifdef IS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

/**
 * URL
 */
void url_init(url_t *url_data) {
    url_t local_url_data;

    local_url_data = *url_data;

    local_url_data.status = URL_STATUS_OK;
    local_url_data.protocol = URL_PROTOCOL_HTTP;
    local_url_data.host = NULL;
    local_url_data.ip = NULL;
    local_url_data.port = NULL;
    local_url_data.path = NULL;
    local_url_data.query = NULL;

    *url_data = local_url_data;
}

unsigned int url_parse(url_t *url_data, char *url) {
    unsigned int rc = RETURN_OK;
    char *token = NULL;
    char *local_url = NULL, *local_url_ptr = NULL;
    char *host_port_token = NULL, *host_port = NULL, *host_port_ptr = NULL;
    struct hostent *he;
    struct in_addr **addr_list;
    url_t local_url_data;

    local_url_data = *url_data;

    // local_url
    local_url = (char *)nessemble_malloc(sizeof(char) * (strlen(url) + 1));
    strcpy(local_url, url);

    token = strtok_r(local_url, "://", &local_url_ptr);

    if (!token) {
        local_url_data.status = URL_STATUS_URI_SYNTAX;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // protocol
    if (strcmp(token, "https") == 0) {
        local_url_data.protocol = URL_PROTOCOL_HTTPS;
    } else if (strcmp(token, "http") == 0) {
        local_url_data.protocol = URL_PROTOCOL_HTTP;
    } else if (strcmp(token, "ftp") == 0) {
        local_url_data.protocol = URL_PROTOCOL_FTP;
    } else {
        local_url_data.protocol = URL_PROTOCOL_OTHER;
    }

    // only http
    if (local_url_data.protocol != URL_PROTOCOL_HTTP) {
        local_url_data.status = URL_STATUS_INVALID_PROTOCOL;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // host:port
    token = strtok_r(NULL, "/", &local_url_ptr);

    if (!token) {
        local_url_data.status = URL_STATUS_HOST_PORT;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    host_port = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 1));
    strcpy(host_port, token);

    // host
    host_port_token = strtok_r(host_port, ":", &host_port_ptr);

    if (!host_port_token) {
        local_url_data.status = URL_STATUS_HOST;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    local_url_data.host = (char *)nessemble_malloc(sizeof(char) * (strlen(host_port_token) + 1));
    strcpy(local_url_data.host, host_port_token);

    // ip
    if ((he = gethostbyname(local_url_data.host)) == NULL) {
        local_url_data.status = URL_STATUS_IP;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    if (addr_list[0]) {
        local_url_data.ip = (char *)nessemble_malloc(sizeof(char) * 16);
        strcpy(local_url_data.ip, inet_ntoa(*addr_list[0]));
    }

    // port
    host_port_token = strtok_r(NULL, ":", &host_port_ptr);

    if (!host_port_token) {
        local_url_data.port = (char *)nessemble_malloc(sizeof(char) * 3);
        strcpy(local_url_data.port, "80");
    } else {
        local_url_data.port = (char *)nessemble_malloc(sizeof(char) * (strlen(host_port_token) + 1));
        strcpy(local_url_data.port, host_port_token);
    }

    // path
    token = strtok_r(NULL, "?", &local_url_ptr);

    if (!token) {
        local_url_data.path = (char *)nessemble_malloc(sizeof(char) * 2);
        strcpy(local_url_data.path, "/");
    } else {
        local_url_data.path = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 2));
        strcpy(local_url_data.path, "/");
        strcat(local_url_data.path, token);
    }

    // query
    token = strtok_r(NULL, "?", &local_url_ptr);

    if (token) {
        local_url_data.query = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(local_url_data.query, token);
    }

cleanup:
    *url_data = local_url_data;

    nessemble_free(local_url);
    nessemble_free(host_port);

    return rc;
}

void url_release(url_t *url_data) {
    url_t local_url_data;

    local_url_data = *url_data;

    nessemble_free(local_url_data.host);
    nessemble_free(local_url_data.ip);
    nessemble_free(local_url_data.port);
    nessemble_free(local_url_data.path);
    nessemble_free(local_url_data.query);

    *url_data = local_url_data;
}

/**
 * HEADERS
 */
void headers_init(http_t *request) {
    unsigned int i = 0, l = 0;
    http_t local_request;

    local_request = *request;

    for (i = 0, l = MAX_HEADERS; i < l; i++) {
        local_request.request_headers[i].key = NULL;
        local_request.request_headers[i].val = NULL;
        local_request.response_headers[i].key = NULL;
        local_request.response_headers[i].val = NULL;
    }

    *request = local_request;
}

void header_release(http_t *request) {
    unsigned int i = 0, l = 0;
    http_t local_request;

    local_request = *request;

    for (i = 0, l = local_request.request_header_count; i < l; i++) {
        nessemble_free(local_request.request_headers[i].key);
        nessemble_free(local_request.request_headers[i].val);
    }

    for (i = 0, l = local_request.response_header_count; i < l; i++) {
        nessemble_free(local_request.response_headers[i].key);
        nessemble_free(local_request.response_headers[i].val);
    }

    *request = local_request;
}

/**
 * HTTP
 */
void http_init(http_t *request) {
    http_t local_request;

    local_request = *request;

    local_request.status = HTTP_STATUS_PENDING;
    local_request.state = HTTP_STATE_READY;

    local_request.socket = -1;
    local_request.method = NULL;
    local_request.method_id = HTTP_METHOD_UNKNOWN;

    local_request.request_size = 0;
    local_request.request_data = NULL;
    local_request.request_body_size = 0;
    local_request.request_body = NULL;

    local_request.status_code = 500;
    local_request.response_line = 0;
    local_request.response_size = 0;
    local_request.content_length = 0;
    local_request.response_data = NULL;
    local_request.response_body = NULL;
    local_request.status_message = NULL;
    local_request.content_type = NULL;

    local_request.response_data = (char *)nessemble_malloc(sizeof(char));

    url_init(&local_request.url_data);

    local_request.request_header_count = 0;
    local_request.response_header_count = 0;

    headers_init(&local_request);

    *request = local_request;
}

static int http_connect(char *host, char *port) {
    int sockfd = -1, res = 0;
    struct addrinfo hints;
    struct addrinfo *addri = 0;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &addri) != 0) {
        sockfd = -1;
        goto cleanup;
    }

    sockfd = socket(addri->ai_family, addri->ai_socktype, addri->ai_protocol);

    if (sockfd == -1) {
        freeaddrinfo(addri);
        goto cleanup;
    }

#ifdef IS_WINDOWS
    u_long nonblocking = 1;
    res = ioctlsocket(sockfd, FIONBIO, &nonblocking);
#else
    int flags = fcntl(sockfd, F_GETFL, 0);
    res = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif

    if (res == -1) {
        freeaddrinfo(addri);

#ifdef IS_WINDOWS
        closesocket(sockfd);
#else
        close(sockfd);
#endif

        sockfd = -1;
        goto cleanup;
    }

    if (connect(sockfd, addri->ai_addr, (int)addri->ai_addrlen) == -1) {

#ifdef IS_WINDOWS
        if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAEINPROGRESS) {
            freeaddrinfo(addri);
            closesocket(sockfd);

            sockfd = -1;
            goto cleanup;
        }
#else
        if (errno != EWOULDBLOCK && errno != EINPROGRESS && errno != EAGAIN) {
            freeaddrinfo(addri);
            close(sockfd);

            sockfd = -1;
            goto cleanup;
        }
#endif
    }

    freeaddrinfo(addri);

cleanup:
    return sockfd;
}

unsigned int http_header(http_t *request, char *key, char *val) {
    unsigned int rc = RETURN_OK;
    http_t local_request;

    local_request = *request;

    local_request.request_headers[local_request.request_header_count].key = (char *)nessemble_malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(local_request.request_headers[local_request.request_header_count].key, key);

    local_request.request_headers[local_request.request_header_count].val = (char *)nessemble_malloc(sizeof(char) * (strlen(val) + 1));
    strcpy(local_request.request_headers[local_request.request_header_count].val, val);

    local_request.request_header_count++;

    *request = local_request;

    return rc;
}

unsigned int http_headers(http_t *request, size_t header_count, char *headers[header_count][2]) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    http_t local_request;

    local_request = *request;

    for (i = 0, l = (unsigned int)header_count; i < l; i++) {
        if ((rc = http_header(&local_request, headers[i][0], headers[i][1])) != 0) {
            goto cleanup;
        }
    }

cleanup:
    *request = local_request;

    return rc;
}

int http_header_cmp(http_t request, char *key, char *val) {
    int cmp = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = request.response_header_count; i < l; i++) {
        if (strcmp(request.response_headers[i].key, key) == 0 && strcmp(request.response_headers[i].val, val) == 0) {
            cmp = 0;
        }
    }

    return cmp;
}

unsigned int http_data(http_t *request, char *data, size_t data_len) {
    unsigned int rc = RETURN_OK;
    char content_length[16];
    http_t local_request;

    local_request = *request;

    // set body + body size
    local_request.request_body_size = data_len;
    local_request.request_body = (char *)nessemble_malloc(sizeof(char) * (data_len + 1));
    memcpy(local_request.request_body, data, data_len);

    // set content-length header
    sprintf(content_length, "%d", (int)data_len);

    if ((rc = http_header(&local_request, "Content-Length", (char *)content_length)) != 0) {
        goto cleanup;
    }

cleanup:
    *request = local_request;

    return rc;
}

unsigned int http_request(http_t *request, char *method, char *url) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    http_t local_request;

    local_request = *request;

#ifdef IS_WINDOWS
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(1, 0), &wsa_data) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }
#endif

    // method
    local_request.method = (char *)nessemble_malloc(sizeof(char) * (strlen(method) + 1));
    strcpy(local_request.method, method);

    if (strcmp(method, "GET") == 0) {
        local_request.method_id = HTTP_METHOD_GET;
    } else if (strcmp(method, "HEAD") == 0) {
        local_request.method_id = HTTP_METHOD_HEAD;
    } else if (strcmp(method, "POST") == 0) {
        local_request.method_id = HTTP_METHOD_POST;
    } else if (strcmp(method, "PUT") == 0) {
        local_request.method_id = HTTP_METHOD_PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        local_request.method_id = HTTP_METHOD_DELETE;
    } else if (strcmp(method, "CONNECT") == 0) {
        local_request.method_id = HTTP_METHOD_CONNECT;
    } else if (strcmp(method, "OPTIONS") == 0) {
        local_request.method_id = HTTP_METHOD_OPTIONS;
    } else if (strcmp(method, "PATCH") == 0) {
        local_request.method_id = HTTP_METHOD_PATCH;
    } else {
        local_request.method_id = HTTP_METHOD_UNKNOWN;
    }

    // parse url
    if ((rc = url_parse(&local_request.url_data, url)) != 0) {
        goto cleanup;
    }

    // connect to socket
    if (local_request.url_data.ip) {
        local_request.socket = http_connect(local_request.url_data.ip, local_request.url_data.port);
    } else {
        local_request.socket = http_connect(local_request.url_data.host, local_request.url_data.port);
    }

    if (local_request.socket < 0) {
        local_request.status = HTTP_STATUS_FAILED;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // create request
    local_request.request_size += strlen(method) + strlen(local_request.url_data.path) + 12; // <method> <path> HTTP/1.1\r\n
    local_request.request_size += strlen(local_request.url_data.host) + strlen(local_request.url_data.port) + 9; // Host: <host>:port\r\n

    for (i = 0, l = local_request.request_header_count; i < l; i++) {
        local_request.request_size += strlen(local_request.request_headers[i].key) + strlen(local_request.request_headers[i].val) + 4; // <key>: <val>\r\n
    }

    local_request.request_size += 2; // \r\n

    if (local_request.request_body_size > 0) {
        local_request.request_size += local_request.request_body_size;
    }

    local_request.request_data = (char *)nessemble_malloc(sizeof(char) * (local_request.request_size + 1));

    sprintf(local_request.request_data, "%s %s HTTP/1.1\r\n", method, local_request.url_data.path);
    sprintf(local_request.request_data+strlen(local_request.request_data), "Host: %s:%s\r\n", local_request.url_data.host, local_request.url_data.port);

    for (i = 0, l = local_request.request_header_count; i < l; i++) {
        sprintf(local_request.request_data+strlen(local_request.request_data), "%s: %s\r\n", local_request.request_headers[i].key, local_request.request_headers[i].val);
    }

    strcat(local_request.request_data, "\r\n");

    if (local_request.request_size > 0) {
        memcpy(local_request.request_data+strlen(local_request.request_data), local_request.request_body, local_request.request_body_size);
    }

cleanup:
    *request = local_request;

    return rc;
}

static unsigned int http_parse_header(http_t *request, char *line, size_t line_len) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char local_line[RESPONSE_BUFFER_LEN];
    char *line_token = NULL, *line_ptr = NULL;
    http_t local_request;

    local_request = *request;

    memset(local_line, '\0', RESPONSE_BUFFER_LEN);
    strncpy(local_line, line, line_len);

    line_token = strtok_r(local_line, ": ", &line_ptr);

    if (!line_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    local_request.response_headers[local_request.response_header_count].key = (char *)nessemble_malloc(sizeof(char) * (strlen(line_token) + 1));
    strcpy(local_request.response_headers[local_request.response_header_count].key, line_token);

    line_token = strtok_r(NULL, "", &line_ptr);

    if (!line_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(line_token+1);
    local_request.response_headers[local_request.response_header_count].val = (char *)nessemble_malloc(sizeof(char) * (length + 1));
    strncpy(local_request.response_headers[local_request.response_header_count].val, line_token+1, length);

    if (strcmp(local_request.response_headers[local_request.response_header_count].key, "Content-Length") == 0) {
        local_request.content_length = (size_t)atoi(local_request.response_headers[local_request.response_header_count].val);
    }

    if (strcmp(local_request.response_headers[local_request.response_header_count].key, "Content-Type") == 0) {
        local_request.content_type = (char *)nessemble_malloc(sizeof(char) * (strlen(local_request.response_headers[local_request.response_header_count].val) + 1));
        strcpy(local_request.content_type, local_request.response_headers[local_request.response_header_count].val);
    }

    local_request.response_header_count++;

cleanup:
    *request = local_request;

    return rc;
}

static unsigned int http_assemble_response(http_t *request, char *line, size_t line_len) {
    unsigned int rc = RETURN_OK;
    http_t local_request;

    local_request = *request;

    local_request.response_data = (char *)realloc(local_request.response_data, sizeof(char) * ((local_request.response_size + line_len) + 1));
    memcpy(local_request.response_data+local_request.response_size, line, line_len);
    local_request.response_size += line_len;

    *request = local_request;

    return rc;
}

unsigned int http_parse(http_t *request) {
    unsigned int rc = RETURN_OK;
    size_t headers_len = 0;
    char *headers = NULL, *headers_break = NULL;
    char *header_token = NULL, *header_ptr = NULL;
    char *status_token = NULL, *status_ptr = NULL;
    http_t local_request;

    local_request = *request;

    // get headers
    headers_break = strstr(local_request.response_data, "\r\n\r\n");

    if (!headers_break) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    headers_len = (size_t)(headers_break - local_request.response_data);
    headers = (char *)nessemble_malloc(sizeof(char) * (headers_len + 1));
    strncpy(headers, local_request.response_data, headers_len);

    // parse status
    header_token = strtok_r(headers, "\r\n", &header_ptr);
    status_token = strtok_r(header_token, " ", &status_ptr);

    if (!status_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (strncmp(status_token, "HTTP/1.", 7) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    status_token = strtok_r(NULL, " ", &status_ptr);

    if (!status_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    local_request.status_code = atoi(status_token);

    status_token = strtok_r(NULL, "", &status_ptr);

    if (!status_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    local_request.status_message = (char *)nessemble_malloc(sizeof(char) * (strlen(status_token) + 1));
    strcpy(local_request.status_message, status_token);

    // parse headers
    header_token = strtok_r(NULL, "\r\n", &header_ptr);

    while (header_token != NULL) {
        if ((rc = http_parse_header(&local_request, header_token, strlen(header_token))) != 0) {
            goto cleanup;
        }

        header_token = strtok_r(NULL, "\r\n", &header_ptr);
    }

    // get body
    if (headers_len+4 < local_request.response_size) {
        local_request.response_body = (char *)nessemble_malloc(sizeof(char) * (local_request.content_length + 1));
        memcpy(local_request.response_body, local_request.response_data+(headers_len+4), local_request.content_length);
    }

cleanup:
    *request = local_request;

    nessemble_free(headers);

    return rc;
}

static unsigned int http_content_length(http_t request) {
    unsigned int content_length = 0;
    char *response_data = NULL;
    char *content_length_start = NULL;
    char *content_length_token = NULL, *content_length_ptr = NULL;

    response_data = (char *)nessemble_malloc(sizeof(char) * (request.response_size + 1));
    strcpy(response_data, request.response_data);

    if ((content_length_start = strstr(response_data, "Content-Length")) == NULL) {
        goto cleanup;
    }

    content_length_token = strtok_r(content_length_start, ": ", &content_length_ptr);

    if (!content_length_token) {
        goto cleanup;
    }

    if (strcmp(content_length_token, "Content-Length") != 0) {
        goto cleanup;
    }

    content_length_token = strtok_r(NULL, "\r\n", &content_length_ptr);

    if (!content_length_token) {
        goto cleanup;
    }

    content_length = (unsigned int)atoi(content_length_token+1);

cleanup:
    nessemble_free(response_data);

    return content_length;
}

http_status_t http_process(http_t *request) {
    size_t headers_len = 0;
    char *headers_break = NULL;
    socklen_t len = 0;
    http_t local_request;

    local_request = *request;

    if (local_request.status == HTTP_STATUS_FAILED) {
        goto cleanup;
    }

    // connect
    if ((local_request.state & HTTP_STATE_CONNECTED) == 0) {
        int opt = 1;
        fd_set sockets_to_check;
        struct timeval timeout;

        FD_ZERO(&sockets_to_check);
        FD_SET(local_request.socket, &sockets_to_check);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        if (select((int)(local_request.socket + 1), NULL, &sockets_to_check, NULL, &timeout) == 1) {
            opt = -1;
            len = sizeof(opt);

            if (getsockopt(local_request.socket, SOL_SOCKET, SO_ERROR, (char *)(&opt), &len) >= 0 && opt == 0) {
                local_request.state |= HTTP_STATE_CONNECTED;
            } else {
                local_request.status = HTTP_STATUS_FAILED;
                goto cleanup;
            }
        }
    }

    if ((local_request.state & HTTP_STATE_CONNECTED) == 0) {
        goto cleanup;
    }

    // send
    if ((local_request.state & HTTP_STATE_SENT) == 0) {
        if (send(local_request.socket, local_request.request_data, (int)local_request.request_size, 0) == -1) {
            local_request.status = HTTP_STATUS_FAILED;
            goto cleanup;
        }

        local_request.state |= HTTP_STATE_SENT;

        goto cleanup;
    }

    // recv
    int size = 0;
    char buffer[RESPONSE_BUFFER_LEN];
    fd_set sockets_to_check;
    struct timeval timeout;

    FD_ZERO(&sockets_to_check);
    FD_SET(local_request.socket, &sockets_to_check);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    while (select((int)(local_request.socket + 1), &sockets_to_check, NULL, NULL, &timeout) == 1) {
        memset(buffer, '\0', RESPONSE_BUFFER_LEN);

        size = recv(local_request.socket, buffer, RESPONSE_BUFFER_LEN, 0);

        if (size == -1) {
            local_request.status = HTTP_STATUS_FAILED;
            goto cleanup;
        } else if (size > 0) {
            if (http_assemble_response(&local_request, (char *)buffer, (size_t)size) != 0) {
                local_request.status = HTTP_STATUS_FAILED;
                goto cleanup;
            }

            if ((headers_break = strstr(local_request.response_data, "\r\n\r\n")) != NULL) {
                headers_len = (size_t)(headers_break - local_request.response_data) + 4;

                if (local_request.response_size - headers_len == http_content_length(local_request)) {
                    local_request.status = HTTP_STATUS_COMPLETED;
                    break;
                }
            }
        } else if (size == 0) {
            if (http_assemble_response(&local_request, (char *)buffer, (size_t)size) != 0) {
                local_request.status = HTTP_STATUS_FAILED;
                goto cleanup;
            }

            local_request.status = HTTP_STATUS_COMPLETED;
            break;
        }
    }

cleanup:
    *request = local_request;

    return local_request.status;
}

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
static void http_spinner_start() {
    #define SPINNER_COUNT 10
    #define SPINNER_DELAY 50000

    unsigned int spinner_index = 0;
    char *spinner[SPINNER_COUNT] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };

    printf("\e[?25l%s", spinner[spinner_index++]);
    fflush(stdout);

    for (;;) {
        usleep(SPINNER_DELAY);

        printf("\b\b%s", spinner[spinner_index]);
        fflush(stdout);

        spinner_index = (spinner_index + 1) % SPINNER_COUNT;
    }
}

static void http_spinner_stop(pid) {
    printf("\b\e[?25h");
    fflush(stdout);

    kill(pid, SIGKILL);
}
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

unsigned int http_do(http_t *request, char *method, char *url) {
    unsigned int rc = RETURN_OK;
    http_t local_request;
    http_status_t status = HTTP_STATUS_PENDING;

    local_request = *request;

    if ((rc = http_header(&local_request, "User-Agent", PROGRAM_NAME "/" PROGRAM_VERSION)) != 0) {
        goto cleanup;
    }

    if ((rc = http_request(&local_request, method, url)) != 0) {
        goto cleanup;
    }

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    pid_t pid = fork();

    if (pid == 0) {
        http_spinner_start();
    } else {
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    while (status == HTTP_STATUS_PENDING) {
        status = http_process(&local_request);
    }

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
        http_spinner_stop(pid);
    }
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    if (status == HTTP_STATUS_FAILED) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = http_parse(&local_request)) != 0) {
        goto cleanup;
    }

cleanup:
    *request = local_request;

    return rc;
}

unsigned int http_get(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;
    http_t local_request;

    local_request = *request;

    if ((rc = http_do(&local_request, "GET", url)) != 0) {
        goto cleanup;
    }

cleanup:
    *request = local_request;

    return rc;
}

unsigned int http_post(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;
    http_t local_request;

    local_request = *request;

    if ((rc = http_do(&local_request, "POST", url)) != 0) {
        goto cleanup;
    }

cleanup:
    *request = local_request;

    return rc;
}

void http_release(http_t *request) {
    http_t local_request;

    local_request = *request;

#ifdef IS_WINDOWS
    closesocket(local_request.socket);
#else
    close(local_request.socket);
#endif

    nessemble_free(local_request.method);
    nessemble_free(local_request.request_data);
    nessemble_free(local_request.request_body);
    nessemble_free(local_request.status_message);
    nessemble_free(local_request.content_type);
    nessemble_free(local_request.response_data);
    nessemble_free(local_request.response_body);

#ifdef IS_WINDOWS
    WSACleanup();
#endif

    url_release(&local_request.url_data);
    header_release(&local_request);

    *request = local_request;
}
