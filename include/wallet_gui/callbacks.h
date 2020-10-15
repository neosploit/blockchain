#ifndef CLIENT_CALLBACKS_H
#define CLIENT_CALLBACKS_H

// Response error messages
#define ERROR_INVALID_REQUEST_JSON_DATA             "Request's body contains invalid or unknown JSON data"
#define ERROR_INVALID_BLOCKCHAIN_UPDATE_REQUEST     "New blockchain is invalid"
#define ERROR_INVALID_REQUESTED_BLOCK_ID            "Block id must be an unsigned non-zero integer"
#define ERROR_REQUESTED_BLOCK_NOT_FOUND             "Requested block was not found"
#define ERROR_INVALID_REQUESTED_TRANSACTION_ID      "Transaction id must be an unsigned non-zero integer"
#define ERROR_REQUESTED_TRANSACTION_NOT_FOUND       "Requested transaction was not found"

// Connection
int callback_connection_check(const struct _u_request *request, struct _u_response *response, void *user_data);

// Nodes
int callback_node_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_node_add(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_node_remove(const struct _u_request *request, struct _u_response *response, void *user_data);

// Blockchain
int callback_blockchain_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_blockchain_update(const struct _u_request *request, struct _u_response *response, void *user_data);

// Blocks
int callback_block_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_block_transactions_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_block_transaction_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);

// Mempool / Transactions
int callback_mempool_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_mempool_add(const struct _u_request *request, struct _u_response *response, void *user_data);
int callback_mempool_remove(const struct _u_request *request, struct _u_response *response, void *user_data);

// Network
int callback_network_relay(const struct _u_request *request, struct _u_response *response, void *user_data);

#endif // CLIENT_CALLBACKS_H