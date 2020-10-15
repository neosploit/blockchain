#ifndef CLIENT_ENDPOINTS_H
#define CLIENT_ENDPOINTS_H

#include "client.h"

// Connection

#define ENDPOINT_CONNECTION_CHECK_HTTP_METHOD "*"
#define ENDPOINT_CONNECTION_CHECK_URL_PATH "/"
struct _u_endpoint *endpoint_connection_check();

// Nodes

#define ENDPOINT_NODE_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_NODE_RETRIEVE_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_retrieve(client_settings_t *client);

#define ENDPOINT_NODE_ADD_HTTP_METHOD "POST"
#define ENDPOINT_NODE_ADD_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_add(client_settings_t *client);

#define ENDPOINT_NODE_REMOVE_HTTP_METHOD "DELETE"
#define ENDPOINT_NODE_REMOVE_URL_PATH "/nodes/"
struct _u_endpoint *endpoint_node_remove(client_settings_t *client);

// Blockchain

#define ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH "/blockchain/"
struct _u_endpoint *endpoint_blockchain_retrieve(client_settings_t *client);

#define ENDPOINT_BLOCKCHAIN_UPDATE_HTTP_METHOD "PUT"
#define ENDPOINT_BLOCKCHAIN_UPDATE_URL_PATH "/blockchain/"
struct _u_endpoint *endpoint_blockchain_update(client_settings_t *client);
struct _u_endpoint *endpoint_blockchain_update_mempool(client_settings_t *client);
struct _u_endpoint *endpoint_blockchain_update_relay(client_settings_t *client);

// Blocks

#define ENDPOINT_BLOCK_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_BLOCK_RETRIEVE_URL_PATH "/blocks/:bid/*"
struct _u_endpoint *endpoint_block_retrieve(client_settings_t *client);

#define ENDPOINT_BLOCK_TRANSACTIONS_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_BLOCK_TRANSACTIONS_RETRIEVE_URL_PATH "/blocks/:bid/transactions/*"
struct _u_endpoint *endpoint_block_transactions_retrieve();

#define ENDPOINT_BLOCK_TRANSACTION_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_BLOCK_TRANSACTION_RETRIEVE_URL_PATH "/blocks/:bid/transactions/:tid/"
struct _u_endpoint *endpoint_block_transaction_retrieve();

// Mempool / Transactions

#define ENDPOINT_MEMPOOL_RETRIEVE_HTTP_METHOD "GET"
#define ENDPOINT_MEMPOOL_RETRIEVE_URL_PATH "/mempool/"
struct _u_endpoint *endpoint_mempool_retrieve(client_settings_t *client);

#define ENDPOINT_MEMPOOL_ADD_HTTP_METHOD "POST"
#define ENDPOINT_MEMPOOL_ADD_URL_PATH "/mempool/"
struct _u_endpoint *endpoint_mempool_add(client_settings_t *client);
struct _u_endpoint *endpoint_mempool_add_relay(client_settings_t *client);

#define ENDPOINT_MEMPOOL_REMOVE_HTTP_METHOD "DELETE"
#define ENDPOINT_MEMPOOL_REMOVE_URL_PATH "/mempool/"
struct _u_endpoint *endpoint_mempool_remove(client_settings_t *client);

#endif // CLIENT_ENDPOINTS_H