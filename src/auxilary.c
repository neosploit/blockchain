#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <ulfius.h>
#include "../include/auxilary.h"

int send_http_request(struct _u_request *request, struct _u_response *response, const char *ip_address, const in_port_t port, const char *http_verb, const char *url_path, json_t *json_body, long request_timeout) {
    int res;

    // Get the request ready
    ulfius_init_request(request);
    ulfius_init_response(response);

    asprintf(&request->http_verb, http_verb);
    asprintf(&request->http_url, "http://%s:%hu%s", ip_address, port, url_path);
    request->timeout = request_timeout;

    // Set request body if needed
    if (json_body && (res = ulfius_set_json_body_request(request, json_body)) != U_OK) {
        fprintf(stderr, "\n[ERROR] send_http_request/ulfius_set_json_body_request (%s): %d\n", request->http_url, res);
        return -1;
    }

    // Send the request
    return ulfius_send_http_request(request, response);
}

bool ip_address_is_valid(const char *ip_address) {
    struct sockaddr_in sa;

    return (inet_pton(AF_INET, ip_address, &(sa.sin_addr)) == 1);
}

bool port_is_valid(json_int_t port) {
    return port > 0 && port < 65536;
}

// https://github.com/littlstar/asprintf.c
int vasprintf(char **str, const char *fmt, va_list args) {
    int size = 0;
    va_list tmpa;

    // copy
    va_copy(tmpa, args);

    // apply variadic arguments to
    // sprintf with format to get size
    size = vsnprintf(NULL, 0, fmt, tmpa);

    // toss args
    va_end(tmpa);

    // return -1 to be compliant if
    // size is less than 0
    if (size < 0) { return -1; }

    // alloc with size plus 1 for `\0'
    *str = (char *) malloc(size + 1);

    // return -1 to be compliant
    // if pointer is `NULL'
    if (NULL == *str) { return -1; }

    // format string with original
    // variadic arguments and set new size
    size = vsprintf(*str, fmt, args);
    return size;
}

// https://github.com/littlstar/asprintf.c
int asprintf(char **str, const char *fmt, ...) {
    int size = 0;
    va_list args;

    // init variadic argumens
    va_start(args, fmt);

    // format and get size
    size = vasprintf(str, fmt, args);

    // toss args
    va_end(args);

    return size;
}
