#ifndef CLIENT_AUXILARY_H
#define CLIENT_AUXILARY_H

#include <stdbool.h>
#include <ulfius.h>
#include "client.h"

int send_http_request(const client_settings_t *client, struct _u_request *request, struct _u_response *response, const char *ip_address, const in_port_t port, const char *http_verb, const char *url_path, json_t *json_body);
bool ip_address_is_valid(const char *ip_address);
bool port_is_valid(json_int_t port);
bool sha256_hash_is_valid(const char *hash);

// https://github.com/littlstar/asprintf.c
int vasprintf(char **, const char *, va_list);
int asprintf (char **, const char *, ...);

#endif // CLIENT_AUXILARY_H