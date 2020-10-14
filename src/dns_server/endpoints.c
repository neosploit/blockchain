#include <stdlib.h>
#include <ulfius.h>
#include "endpoints.h"
#include "callbacks.h"

// Connection

struct _u_endpoint *endpoint_connection_check() {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_CONNECTION_CHECK_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_CONNECTION_CHECK_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_connection_check;

    return endpoint;
}

// Nodes

struct _u_endpoint *endpoint_node_retrieve(server_settings_t *server) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_RETRIEVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_retrieve;
    endpoint->user_data = server;

    return endpoint;
}

struct _u_endpoint *endpoint_node_add(server_settings_t *server) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_ADD_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_ADD_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_add;
    endpoint->user_data = server;

    return endpoint;
}

struct _u_endpoint *endpoint_node_remove(server_settings_t *server) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_REMOVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_REMOVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_remove;
    endpoint->user_data = server;

    return endpoint;
}