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
    url_data->status = URL_STATUS_OK;
    url_data->protocol = URL_PROTOCOL_HTTP;
    url_data->host = NULL;
    url_data->ip = NULL;
    url_data->port = NULL;
    url_data->path = NULL;
    url_data->query = NULL;
}

unsigned int url_parse(url_t *url_data, char *url) {
    unsigned int rc = RETURN_OK;
    char *token = NULL;
    char *local_url = NULL, *local_url_ptr = NULL;
    char *host_port_token = NULL, *host_port = NULL, *host_port_ptr = NULL;
    struct hostent *he;
    struct in_addr **addr_list;

    // local_url
    local_url = (char *)nessemble_malloc(sizeof(char) * (strlen(url) + 1));
    strcpy(local_url, url);

    token = strtok_r(local_url, "://", &local_url_ptr);

    if (!token) {
        url_data->status = URL_STATUS_URI_SYNTAX;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // protocol
    if (strcmp(token, "https") == 0) {
        url_data->protocol = URL_PROTOCOL_HTTPS;
    } else if (strcmp(token, "http") == 0) {
        url_data->protocol = URL_PROTOCOL_HTTP;
    } else if (strcmp(token, "ftp") == 0) {
        url_data->protocol = URL_PROTOCOL_FTP;
    } else {
        url_data->protocol = URL_PROTOCOL_OTHER;
    }

    // only http
    if (url_data->protocol != URL_PROTOCOL_HTTP) {
        url_data->status = URL_STATUS_INVALID_PROTOCOL;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // host:port
    token = strtok_r(NULL, "/", &local_url_ptr);

    if (!token) {
        url_data->status = URL_STATUS_HOST_PORT;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    host_port = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 1));
    strcpy(host_port, token);

    // host
    host_port_token = strtok_r(host_port, ":", &host_port_ptr);

    if (!host_port_token) {
        url_data->status = URL_STATUS_HOST;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    url_data->host = (char *)nessemble_malloc(sizeof(char) * (strlen(host_port_token) + 1));
    strcpy(url_data->host, host_port_token);

    // ip
    if ((he = gethostbyname(url_data->host)) == NULL) {
        url_data->status = URL_STATUS_IP;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    if (addr_list[0]) {
        url_data->ip = (char *)nessemble_malloc(sizeof(char) * 16);
        strcpy(url_data->ip, inet_ntoa(*addr_list[0]));
    }

    // port
    host_port_token = strtok_r(NULL, ":", &host_port_ptr);

    if (!host_port_token) {
        url_data->port = (char *)nessemble_malloc(sizeof(char) * 3);
        strcpy(url_data->port, "80");
    } else {
        url_data->port = (char *)nessemble_malloc(sizeof(char) * (strlen(host_port_token) + 1));
        strcpy(url_data->port, host_port_token);
    }

    // path
    token = strtok_r(NULL, "?", &local_url_ptr);

    if (!token) {
        url_data->path = (char *)nessemble_malloc(sizeof(char) * 2);
        strcpy(url_data->path, "/");
    } else {
        url_data->path = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 2));
        strcpy(url_data->path, "/");
        strcat(url_data->path, token);
    }

    // query
    token = strtok_r(NULL, "?", &local_url_ptr);

    if (token) {
        url_data->query = (char *)nessemble_malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(url_data->query, token);
    }

cleanup:
    nessemble_free(local_url);
    nessemble_free(host_port);

    return rc;
}

void url_release(url_t *url_data) {
    nessemble_free(url_data->host);
    nessemble_free(url_data->ip);
    nessemble_free(url_data->port);
    nessemble_free(url_data->path);
    nessemble_free(url_data->query);
}

/**
 * HEADERS
 */
void headers_init(http_t *request) {
    unsigned int i = 0, l = 0;

    for (i = 0, l = MAX_HEADERS; i < l; i++) {
        request->request_headers[i].key = NULL;
        request->request_headers[i].val = NULL;
        request->response_headers[i].key = NULL;
        request->response_headers[i].val = NULL;
    }
}

void header_release(http_t *request) {
    unsigned int i = 0, l = 0;

    for (i = 0, l = request->request_header_count; i < l; i++) {
        nessemble_free(request->request_headers[i].key);
        nessemble_free(request->request_headers[i].val);
    }

    for (i = 0, l = request->response_header_count; i < l; i++) {
        nessemble_free(request->response_headers[i].key);
        nessemble_free(request->response_headers[i].val);
    }
}

/**
 * HTTP
 */
void http_init(http_t *request) {
    request->status = HTTP_STATUS_PENDING;
    request->state = HTTP_STATE_READY;

    request->socket = -1;
    request->method = NULL;
    request->method_id = HTTP_METHOD_UNKNOWN;

    request->request_size = 0;
    request->request_data = NULL;
    request->request_body_size = 0;
    request->request_body = NULL;

    request->status_code = 500;
    request->response_line = 0;
    request->response_size = 0;
    request->content_length = 0;
    request->response_data = NULL;
    request->response_body = NULL;
    request->status_message = NULL;
    request->content_type = NULL;

    request->response_data = (char *)nessemble_malloc(sizeof(char));

    url_init(&(request->url_data));

    request->request_header_count = 0;
    request->response_header_count = 0;

    headers_init(request);
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

    if (!addri) {
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

    request->request_headers[request->request_header_count].key = (char *)nessemble_malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(request->request_headers[request->request_header_count].key, key);

    request->request_headers[request->request_header_count].val = (char *)nessemble_malloc(sizeof(char) * (strlen(val) + 1));
    strcpy(request->request_headers[request->request_header_count].val, val);

    request->request_header_count++;

    return rc;
}

unsigned int http_headers(http_t *request, size_t header_count, char *headers[header_count][2]) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;

    for (i = 0, l = (unsigned int)header_count; i < l; i++) {
        if ((rc = http_header(request, headers[i][0], headers[i][1])) != 0) {
            goto cleanup;
        }
    }

cleanup:
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

    // set body + body size
    request->request_body_size = data_len;
    request->request_body = (char *)nessemble_malloc(sizeof(char) * (data_len + 1));
    memcpy(request->request_body, data, data_len);

    // set content-length header
    sprintf(content_length, "%d", (int)data_len);

    if ((rc = http_header(request, "Content-Length", (char *)content_length)) != 0) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int http_request(http_t *request, char *method, char *url) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;

#ifdef IS_WINDOWS
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(1, 0), &wsa_data) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }
#endif

    // method
    request->method = (char *)nessemble_malloc(sizeof(char) * (strlen(method) + 1));
    strcpy(request->method, method);

    if (strcmp(method, "GET") == 0) {
        request->method_id = HTTP_METHOD_GET;
    } else if (strcmp(method, "HEAD") == 0) {
        request->method_id = HTTP_METHOD_HEAD;
    } else if (strcmp(method, "POST") == 0) {
        request->method_id = HTTP_METHOD_POST;
    } else if (strcmp(method, "PUT") == 0) {
        request->method_id = HTTP_METHOD_PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        request->method_id = HTTP_METHOD_DELETE;
    } else if (strcmp(method, "CONNECT") == 0) {
        request->method_id = HTTP_METHOD_CONNECT;
    } else if (strcmp(method, "OPTIONS") == 0) {
        request->method_id = HTTP_METHOD_OPTIONS;
    } else if (strcmp(method, "PATCH") == 0) {
        request->method_id = HTTP_METHOD_PATCH;
    } else {
        request->method_id = HTTP_METHOD_UNKNOWN;
    }

    // parse url
    if ((rc = url_parse(&(request->url_data), url)) != 0) {
        goto cleanup;
    }

    // connect to socket
    if (request->url_data.ip) {
        request->socket = http_connect(request->url_data.ip, request->url_data.port);
    } else {
        request->socket = http_connect(request->url_data.host, request->url_data.port);
    }

    if (request->socket < 0) {
        request->status = HTTP_STATUS_FAILED;

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // create request
    request->request_size += strlen(method) + strlen(request->url_data.path) + 12; // <method> <path> HTTP/1.1\r\n
    request->request_size += strlen(request->url_data.host) + strlen(request->url_data.port) + 9; // Host: <host>:port\r\n

    for (i = 0, l = request->request_header_count; i < l; i++) {
        request->request_size += strlen(request->request_headers[i].key) + strlen(request->request_headers[i].val) + 4; // <key>: <val>\r\n
    }

    request->request_size += 2; // \r\n

    if (request->request_body_size > 0) {
        request->request_size += request->request_body_size;
    }

    request->request_data = (char *)nessemble_malloc(sizeof(char) * (request->request_size + 1));

    sprintf(request->request_data, "%s %s HTTP/1.1\r\n", method, request->url_data.path);
    sprintf(request->request_data+strlen(request->request_data), "Host: %s:%s\r\n", request->url_data.host, request->url_data.port);

    for (i = 0, l = request->request_header_count; i < l; i++) {
        sprintf(request->request_data+strlen(request->request_data), "%s: %s\r\n", request->request_headers[i].key, request->request_headers[i].val);
    }

    strcat(request->request_data, "\r\n");

    if (request->request_size > 0) {
        memcpy(request->request_data+strlen(request->request_data), request->request_body, request->request_body_size);
    }

cleanup:
    return rc;
}

static unsigned int http_parse_header(http_t *request, char *line, size_t line_len) {
    unsigned int rc = RETURN_OK;
    size_t length = 0;
    char local_line[RESPONSE_BUFFER_LEN];
    char *line_token = NULL, *line_ptr = NULL;

    memset(local_line, '\0', RESPONSE_BUFFER_LEN);
    strncpy(local_line, line, line_len);

    line_token = strtok_r(local_line, ": ", &line_ptr);

    if (!line_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    request->response_headers[request->response_header_count].key = (char *)nessemble_malloc(sizeof(char) * (strlen(line_token) + 1));
    strcpy(request->response_headers[request->response_header_count].key, line_token);

    line_token = strtok_r(NULL, "", &line_ptr);

    if (!line_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(line_token+1);
    request->response_headers[request->response_header_count].val = (char *)nessemble_malloc(sizeof(char) * (length + 1));
    strncpy(request->response_headers[request->response_header_count].val, line_token+1, length);

    if (strcmp(request->response_headers[request->response_header_count].key, "Content-Length") == 0) {
        request->content_length = (size_t)atoi(request->response_headers[request->response_header_count].val);
    }

    if (strcmp(request->response_headers[request->response_header_count].key, "Content-Type") == 0) {
        request->content_type = (char *)nessemble_malloc(sizeof(char) * (strlen(request->response_headers[request->response_header_count].val) + 1));
        strcpy(request->content_type, request->response_headers[request->response_header_count].val);
    }

    request->response_header_count++;

cleanup:
    return rc;
}

static unsigned int http_assemble_response(http_t *request, char *line, size_t line_len) {
    unsigned int rc = RETURN_OK;
    char *new_response_data = NULL;

    new_response_data = (char *)nessemble_malloc(sizeof(char) * ((request->response_size + line_len) + 1));
    memcpy(new_response_data, request->response_data, request->response_size);
    nessemble_free(request->response_data);

    request->response_data = new_response_data;

    memcpy(request->response_data+request->response_size, line, line_len);
    request->response_size += line_len;

    return rc;
}

unsigned int http_parse(http_t *request) {
    unsigned int rc = RETURN_OK;
    size_t headers_len = 0;
    char *headers = NULL, *headers_break = NULL;
    char *header_token = NULL, *header_ptr = NULL;
    char *status_token = NULL, *status_ptr = NULL;

    // get headers
    headers_break = strstr(request->response_data, "\r\n\r\n");

    if (!headers_break) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    headers_len = (size_t)(headers_break - request->response_data);
    headers = (char *)nessemble_malloc(sizeof(char) * (headers_len + 1));
    strncpy(headers, request->response_data, headers_len);

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

    request->status_code = atoi(status_token);

    status_token = strtok_r(NULL, "", &status_ptr);

    if (!status_token) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    request->status_message = (char *)nessemble_malloc(sizeof(char) * (strlen(status_token) + 1));
    strcpy(request->status_message, status_token);

    // parse headers
    header_token = strtok_r(NULL, "\r\n", &header_ptr);

    while (header_token != NULL) {
        if ((rc = http_parse_header(request, header_token, strlen(header_token))) != 0) {
            goto cleanup;
        }

        header_token = strtok_r(NULL, "\r\n", &header_ptr);
    }

    // get body
    if (headers_len+4 < request->response_size) {
        request->response_body = (char *)nessemble_malloc(sizeof(char) * (request->content_length + 1));
        memcpy(request->response_body, request->response_data+(headers_len+4), request->content_length);
    }

cleanup:
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

    if (request->status == HTTP_STATUS_FAILED) {
        goto cleanup;
    }

    // connect
    if ((request->state & HTTP_STATE_CONNECTED) == 0) {
        int opt = 1;
        fd_set sockets_to_check;
        struct timeval timeout;

        FD_ZERO(&sockets_to_check);
        FD_SET(request->socket, &sockets_to_check);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        if (select((int)(request->socket + 1), NULL, &sockets_to_check, NULL, &timeout) == 1) {
            opt = -1;
            len = sizeof(opt);

            if (getsockopt(request->socket, SOL_SOCKET, SO_ERROR, (char *)(&opt), &len) >= 0 && opt == 0) {
                request->state |= HTTP_STATE_CONNECTED;
            } else {
                request->status = HTTP_STATUS_FAILED;
                goto cleanup;
            }
        }
    }

    if ((request->state & HTTP_STATE_CONNECTED) == 0) {
        goto cleanup;
    }

    // send
    if ((request->state & HTTP_STATE_SENT) == 0) {
        if (send(request->socket, request->request_data, (int)request->request_size, 0) == -1) {
            request->status = HTTP_STATUS_FAILED;
            goto cleanup;
        }

        request->state |= HTTP_STATE_SENT;

        goto cleanup;
    }

    // recv
    int size = 0;
    char buffer[RESPONSE_BUFFER_LEN];
    fd_set sockets_to_check;
    struct timeval timeout;

    FD_ZERO(&sockets_to_check);
    FD_SET(request->socket, &sockets_to_check);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    while (select((int)(request->socket + 1), &sockets_to_check, NULL, NULL, &timeout) == 1) {
        memset(buffer, '\0', RESPONSE_BUFFER_LEN);

        size = recv(request->socket, buffer, RESPONSE_BUFFER_LEN, 0);

        if (size == -1) {
            request->status = HTTP_STATUS_FAILED;
            goto cleanup;
        } else if (size > 0) {
            if (http_assemble_response(request, (char *)buffer, (size_t)size) != 0) {
                request->status = HTTP_STATUS_FAILED;
                goto cleanup;
            }

            if ((headers_break = strstr(request->response_data, "\r\n\r\n")) != NULL) {
                headers_len = (size_t)(headers_break - request->response_data) + 4;

                if (request->response_size - headers_len == http_content_length(*request)) {
                    request->status = HTTP_STATUS_COMPLETED;
                    break;
                }
            }
        } else if (size == 0) {
            if (http_assemble_response(request, (char *)buffer, (size_t)size) != 0) {
                request->status = HTTP_STATUS_FAILED;
                goto cleanup;
            }

            request->status = HTTP_STATUS_COMPLETED;
            break;
        }
    }

cleanup:
    return request->status;
}

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
static void http_spinner_stop(pid_t pid) {
    fprintf(stderr, "\b\e[?25h");
    fflush(stdout);

    kill(pid, SIGKILL);
}

static void http_spinner_start(pid_t pid) {
    #define SPINNER_COUNT 10
    #define SPINNER_DELAY 50000

    unsigned int i = 0, l = 0, spinner_index = 0;
    char *spinner[SPINNER_COUNT] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };
    struct sigaction sa;

    // remove handlers on forked process
    memset(&sa, '\0', sizeof(struct sigaction));

    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = SA_SIGINFO;

    for (i = 1, l = 32; i < l; i++) {
        sigaction(i, &sa, NULL);
    }

    // start spinner
    fprintf(stderr, "\e[?25l%s", spinner[spinner_index++]);
    fflush(stdout);

    for (;;) {
        if (getppid() == 1) {
            http_spinner_stop(pid);
        }

        usleep(SPINNER_DELAY);

        if (getppid() == 1) {
            http_spinner_stop(pid);
        }

        fprintf(stderr, "\b%s", spinner[spinner_index]);
        fflush(stdout);

        spinner_index = (spinner_index + 1) % SPINNER_COUNT;
    }
}
#else /* !IS_WINDOWS && !IS_JAVASCRIPT */
#ifdef IS_WINDOWS
#define SPINNER_COUNT 4
#define SPINNER_DELAY 100000

http_t http_thread_request;
http_status_t http_thread_status;

static void http_thread_spinner_stop() {
    printf("\b\e[?25h");
    fflush(stdout);
}

DWORD WINAPI http_thread_spinner_start() {
    unsigned int spinner_index = 0;
    char spinner[SPINNER_COUNT] = { 0xC4, '\\', 0xB3, '/' };

    // start spinner
    printf("\e[?25l%c", spinner[spinner_index++]);
    fflush(stdout);

    for (;;) {
        if (http_thread_status != HTTP_STATUS_PENDING) {
            break;
        }

        usleep(SPINNER_DELAY);

        if (http_thread_status != HTTP_STATUS_PENDING) {
            break;
        }

        printf("\b%c", spinner[spinner_index]);
        fflush(stdout);

        spinner_index = (spinner_index + 1) % SPINNER_COUNT;
    }

    return 0;
}

DWORD WINAPI http_thread_process() {
    while (http_thread_status == HTTP_STATUS_PENDING) {
        http_thread_status = http_process(&http_thread_request);
    }

    return 0;
}
#endif /* IS_WINDOWS */
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

unsigned int http_do(http_t *request, char *method, char *url) {
    unsigned int rc = RETURN_OK;
    http_status_t status = HTTP_STATUS_PENDING;

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    pid_t pid = -1;
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    if ((rc = http_header(request, "User-Agent", PROGRAM_NAME "/" PROGRAM_VERSION)) != 0) {
        goto cleanup;
    }

    if ((rc = http_request(request, method, url)) != 0) {
        goto cleanup;
    }

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    pid = fork();

    if (pid == 0) {
        http_spinner_start(pid);
    } else {
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

#ifndef IS_WINDOWS
    while (status == HTTP_STATUS_PENDING) {
        status = http_process(request);
    }
#else /* IS_WINDOWS */
    HANDLE http_thread_array[2];

    http_thread_request = *request;
    http_thread_status = HTTP_STATUS_PENDING;

    http_thread_array[0] = CreateThread(NULL, 0, http_thread_spinner_start, NULL, 0, NULL);
    http_thread_array[1] = CreateThread(NULL, 0, http_thread_process, NULL, 0, NULL);

    WaitForMultipleObjects(2, http_thread_array, TRUE, INFINITE);

    http_thread_spinner_stop();

    *request = http_thread_request;
    status = http_thread_status;
#endif /* IS_WINDOWS */

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    }
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    if (status == HTTP_STATUS_FAILED) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((rc = http_parse(request)) != 0) {
        goto cleanup;
    }

cleanup:
#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    if (pid != -1) {
        http_spinner_stop(pid);
    }
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    return rc;
}

unsigned int http_head(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;

    if ((rc = http_do(request, "HEAD", url)) != 0) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int http_get(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;

    if ((rc = http_do(request, "GET", url)) != 0) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int http_post(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;

    if ((rc = http_do(request, "POST", url)) != 0) {
        goto cleanup;
    }

cleanup:
    return rc;
}

unsigned int http_put(http_t *request, char *url) {
    unsigned int rc = RETURN_OK;

    if ((rc = http_do(request, "PUT", url)) != 0) {
        goto cleanup;
    }

cleanup:
    return rc;
}

void http_release(http_t *request) {
#ifdef IS_WINDOWS
    closesocket(request->socket);
#else
    close(request->socket);
#endif

    nessemble_free(request->method);
    nessemble_free(request->request_data);
    nessemble_free(request->request_body);
    nessemble_free(request->status_message);
    nessemble_free(request->content_type);
    nessemble_free(request->response_data);
    nessemble_free(request->response_body);

#ifdef IS_WINDOWS
    WSACleanup();
#endif

    url_release(&(request->url_data));
    header_release(request);
}
