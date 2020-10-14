#ifndef SERVER_CALLBACKS_H
#define SERVER_CALLBACKS_H

// Response error messages
#define ERROR_INVALID_REQUEST_JSON_DATA             "Request's body contains invalid or unknown JSON data"

// Connection
int callback_connection_check(const struct _u_request *request, struct _u_response *response, void *user_data);

// Nodes
int callback_node_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_node_add(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_node_remove(const struct _u_request *request, struct _u_response *response, void *user_data);

#endif // SERVER_CALLBACKS_H