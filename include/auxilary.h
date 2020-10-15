#ifndef CLIENT_AUXILARY_H
#define CLIENT_AUXILARY_H

#include <netinet/in.h>
#include <stdbool.h>
#include <ulfius.h>

int send_http_request(struct _u_request *request, struct _u_response *response, const char *ip_address, const in_port_t port, const char *http_verb, const char *url_path, json_t *json_body, long request_timeout);
bool ip_address_is_valid(const char *ip_address);
bool port_is_valid(json_int_t port);

// https://github.com/littlstar/asprintf.c
int vasprintf(char **, const char *, va_list);
int asprintf (char **, const char *, ...);

#endif // CLIENT_AUXILARY_H