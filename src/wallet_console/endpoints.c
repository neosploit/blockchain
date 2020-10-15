#include <stdlib.h>
#include <ulfius.h>
#include "../client/endpoints.h"
#include "../client/callbacks.h"

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

struct _u_endpoint *endpoint_node_retrieve(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_RETRIEVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_retrieve;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_node_add(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_ADD_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_ADD_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_add;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_node_remove(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_NODE_REMOVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_NODE_REMOVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_node_remove;
    endpoint->user_data = client;

    return endpoint;
}

// Blockchain

struct _u_endpoint *endpoint_blockchain_retrieve(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_blockchain_retrieve;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_blockchain_update(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCKCHAIN_UPDATE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCKCHAIN_UPDATE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_blockchain_update;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_blockchain_update_mempool(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCKCHAIN_UPDATE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCKCHAIN_UPDATE_URL_PATH;
    endpoint->priority = 1;
    endpoint->callback_function = &callback_mempool_remove;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_blockchain_update_relay(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCKCHAIN_UPDATE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCKCHAIN_UPDATE_URL_PATH;
    endpoint->priority = 2;
    endpoint->callback_function = &callback_network_relay;
    endpoint->user_data = client;

    return endpoint;
}

// Blocks

struct _u_endpoint *endpoint_block_retrieve(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCK_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCK_RETRIEVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_block_retrieve;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_block_transactions_retrieve() {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCK_TRANSACTIONS_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCK_TRANSACTIONS_RETRIEVE_URL_PATH;
    endpoint->priority = 1;
    endpoint->callback_function = &callback_block_transactions_retrieve;

    return endpoint;
}

struct _u_endpoint *endpoint_block_transaction_retrieve() {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_BLOCK_TRANSACTION_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_BLOCK_TRANSACTION_RETRIEVE_URL_PATH;
    endpoint->priority = 2;
    endpoint->callback_function = &callback_block_transaction_retrieve;

    return endpoint;
}

// Mempool / Transactions

struct _u_endpoint *endpoint_mempool_retrieve(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_MEMPOOL_RETRIEVE_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_MEMPOOL_RETRIEVE_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_mempool_retrieve;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_mempool_add(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_MEMPOOL_ADD_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_MEMPOOL_ADD_URL_PATH;
    endpoint->priority = 0;
    endpoint->callback_function = &callback_mempool_add;
    endpoint->user_data = client;

    return endpoint;
}

struct _u_endpoint *endpoint_mempool_add_relay(client_settings_t *client) {
    struct _u_endpoint *endpoint = malloc(sizeof(struct _u_endpoint));

    endpoint->http_method = ENDPOINT_MEMPOOL_ADD_HTTP_METHOD;
    endpoint->url_format = ENDPOINT_MEMPOOL_ADD_URL_PATH;
    endpoint->priority = 1;
    endpoint->callback_function = &callback_network_relay;
    endpoint->user_data = client;

    return endpoint;
}