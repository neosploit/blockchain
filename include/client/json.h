#ifndef CLIENT_OBJECTS_H
#define CLIENT_OBJECTS_H

#include <stdbool.h>
#include <netinet/in.h>

typedef enum json_object_type_t {
    NODE,
    TRANSACTION,
    BLOCK,
    WALLET
} json_object_type_t;

// Node

typedef struct node_t {
    const char *ip_address;
    in_port_t port;
} node_t;

json_t *json_construct_node(const node_t *node);
node_t json_destruct_node(const json_t *json_node);
bool json_compare_nodes(const json_t *json_node1, const json_t *json_node2);
bool json_node_is_valid(const json_t *json_node);

// Transaction

typedef struct transaction_t {
    unsigned long int timestamp;
    const char *sender;
    const char *receiver;
    double amount;
} transaction_t;

json_t *json_construct_transaction(const transaction_t *transaction);
transaction_t json_destruct_transaction(const json_t *json_transaction);
bool json_compare_transactions(const json_t *json_transaction1, const json_t *json_transaction2);
bool json_transaction_is_valid(const json_t *json_transaction);

// Block

typedef struct block_t {
    unsigned long int timestamp;
    unsigned int id;
    unsigned int nonce;
    double coinbase;
    unsigned int transaction_count;
    transaction_t *transactions;
    const char *previous;
    const char *hash;
} block_t;

json_t *json_construct_block(const block_t *block);
block_t json_destruct_block(const json_t *json_block);
bool json_compare_blocks(const json_t *json_block1, const json_t *json_block2);
bool json_block_is_valid(const json_t *json_block);

// Wallet

typedef struct wallet_t {
    const char *name;
    const char *private_key;
    const char *public_key;
    const char *address;
} wallet_t;

json_t *json_construct_wallet(const wallet_t *wallet);
wallet_t json_destruct_wallet(const json_t *json_wallet);
bool json_wallet_is_valid(const json_t *json_wallet);

// Extra

bool json_request_is_valid(const json_t *body, json_object_type_t object_type);
bool json_array_is_valid(const json_t *array, json_object_type_t object_type);
bool json_object_is_valid(const json_t *object, json_object_type_t object_type);

#endif // CLIENT_OBJECTS_H