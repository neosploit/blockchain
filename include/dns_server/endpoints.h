#ifndef SERVER_ENDPOINTS_H
#define SERVER_ENDPOINTS_H

#include "server.h"

// Connection

#define ENDPOINT_CONNECTION_CHECK_HTTP_METHOD "*"
#define ENDPOINT_CONNECTION_CHECK_URL_PATH "/"
struct _u_endpoint *endpoint_connection_check();

// Nodes

#define ENDPOINT_NODE_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_NODE_RETRIEVE_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_retrieve(server_settings_t *server);

#define ENDPOINT_NODE_ADD_HTTP_METHOD "POST"
#define ENDPOINT_NODE_ADD_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_add(server_settings_t *server);

#define ENDPOINT_NODE_REMOVE_HTTP_METHOD "DELETE"
#define ENDPOINT_NODE_REMOVE_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_remove(server_settings_t *server);

#endif // SERVER_ENDPOINTS_H