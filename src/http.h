#ifndef _HTTP_H
#define _HTTP_H

#define MAX_HEADERS         32
#define RESPONSE_BUFFER_LEN 4096

/**
 * URL
 */
typedef enum url_status_t {
    URL_STATUS_OK,
    URL_STATUS_INVALID_PROTOCOL,
    URL_STATUS_URI_SYNTAX,
    URL_STATUS_HOST_PORT,
    URL_STATUS_HOST,
    URL_STATUS_IP
} url_status_t;

typedef enum url_protocol_t {
    URL_PROTOCOL_OTHER,
    URL_PROTOCOL_HTTP,
    URL_PROTOCOL_HTTPS,
    URL_PROTOCOL_FTP
} url_protocol_t;

typedef struct url_t {
    url_status_t status;
    url_protocol_t protocol;
    char *host;
    char *ip;
    char *port;
    char *path;
    char *query;
} url_t;

void url_init(url_t *url_data);
unsigned int url_parse(url_t *url_data, char *url);

/**
 * HEADERS
 */
typedef struct header_t {
    char *key;
    char *val;
} header_t;

/**
 * HTTP
 */
typedef enum http_status_t {
    HTTP_STATUS_PENDING,
    HTTP_STATUS_COMPLETED,
    HTTP_STATUS_FAILED
} http_status_t;

typedef enum http_state_t {
    HTTP_STATE_READY     = 0x00,
    HTTP_STATE_CONNECTED = 0x01,
    HTTP_STATE_SENT      = 0x02,
    HTTP_STATE_PARSE     = 0x04
} http_state_t;

typedef enum http_method_t {
    HTTP_METHOD_UNKNOWN,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_PATCH
} http_method_t;

typedef struct http_t {
    // status
    http_status_t status;
    http_state_t state;

    // connection settings
    int socket;
    char *method;
    http_method_t method_id;

    // request info
    size_t request_size;
    char *request_data;
    size_t request_body_size;
    char *request_body;

    // response info
    unsigned int status_code;
    unsigned int response_line;
    size_t response_size;
    size_t content_length;
    char *response_data;
    char *response_body;
    char *status_message;
    char *content_type;

    // url data
    url_t url_data;

    // header data
    unsigned int request_header_count;
    header_t request_headers[MAX_HEADERS];
    unsigned int response_header_count;
    header_t response_headers[MAX_HEADERS];
} http_t;

void http_init(http_t *request);
unsigned int http_header(http_t *request, char *key, char *val);
unsigned int http_headers(http_t *request, size_t header_count, char *headers[header_count][2]);
int http_header_cmp(http_t request, char *key, char *val);
unsigned int http_data(http_t *request, char *data, size_t data_len);
unsigned int http_request(http_t *request, char *method, char *url);
http_status_t http_process(http_t *request);
unsigned int http_parse(http_t *request);
unsigned int http_get(http_t *request, char *url);
unsigned int http_post(http_t *request, char *url);
void http_release(http_t *request);

#endif /* _HTTP_H */
