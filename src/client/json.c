#include <string.h>
#include <jansson.h>
#include "auxilary.h"
#include "json.h"

// Node

json_t *json_construct_node(const node_t *node) {
    json_t *json_node = json_object();

    if (json_object_set(json_node, "ip_address", json_string(node->ip_address)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_node/json_object_set (ip_address)\n");
        return NULL;
    }

    if (json_object_set(json_node, "port", json_integer(node->port)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_node/json_object_set (port)\n");
        return NULL;
    }

    return json_node;
}

node_t json_destruct_node(const json_t *json_node) {
    node_t node;

    node.ip_address = json_string_value(json_object_get(json_node, "ip_address"));
    node.port = json_integer_value(json_object_get(json_node, "port"));

    return node;
}

bool json_compare_nodes(const json_t *json_node1, const json_t *json_node2) {
    const node_t node1 = json_destruct_node(json_node1);
    const node_t node2 = json_destruct_node(json_node2);

    if (strcmp(node1.ip_address, node2.ip_address)) return false;
    if (node1.port != node2.port) return false;

    return true;
}

bool json_node_is_valid(const json_t *json_node) {
    if (!json_is_object(json_node)) return false;
    if (json_object_size(json_node) != 2) return false;

    // 1: IP address
    if (!json_object_get(json_node, "ip_address")) return false;
    if (!json_is_string(json_object_get(json_node, "ip_address"))) return false;
    if (!ip_address_is_valid(json_string_value(json_object_get(json_node, "ip_address")))) return false;

    // 2: Port
    if (!json_object_get(json_node, "port")) return false;
    if (!json_is_integer(json_object_get(json_node, "port"))) return false;
    if (!port_is_valid(json_integer_value(json_object_get(json_node, "port")))) return false;

    return true;
}

// Transaction

json_t *json_construct_transaction(const transaction_t *transaction) {
    json_t *json_transaction = json_object();

    if (json_object_set(json_transaction, "timestamp", json_integer(transaction->timestamp)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_transaction/json_object_set (timestamp)\n");
        return NULL;
    }

    if (json_object_set(json_transaction, "sender", json_string(transaction->sender)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_transaction/json_object_set (sender)\n");
        return NULL;
    }

    if (json_object_set(json_transaction, "receiver", json_string(transaction->receiver)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_transaction/json_object_set (receiver)\n");
        return NULL;
    }

    if (json_object_set(json_transaction, "amount", json_real(transaction->amount)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_transaction/json_object_set (amount)\n");
        return NULL;
    }

    return json_transaction;
}

transaction_t json_destruct_transaction(const json_t *json_transaction) {
    transaction_t transaction;

    transaction.timestamp = json_integer_value(json_object_get(json_transaction, "timestamp"));
    transaction.sender = json_string_value(json_object_get(json_transaction, "sender"));
    transaction.receiver = json_string_value(json_object_get(json_transaction, "receiver"));
    transaction.amount = json_real_value(json_object_get(json_transaction, "amount"));

    return transaction;
}

bool json_compare_transactions(const json_t *json_transaction1, const json_t *json_transaction2) {
    const transaction_t transaction1 = json_destruct_transaction(json_transaction1);
    const transaction_t transaction2 = json_destruct_transaction(json_transaction2);

    if (transaction1.timestamp != transaction2.timestamp) return false;
    if (strcmp(transaction1.sender, transaction2.sender)) return false;
    if (strcmp(transaction1.receiver, transaction2.receiver)) return false;
    if (transaction1.amount != transaction2.amount) return false;

    return true;
}

bool json_transaction_is_valid(const json_t *json_transaction) {
    if (!json_is_object(json_transaction)) return false;
    if (json_object_size(json_transaction) != 4) return false;
    
    // 1: Timestamp
    if (!json_object_get(json_transaction, "timestamp")) return false;
    if (!json_is_integer(json_object_get(json_transaction, "timestamp"))) return false;
    if (json_integer_value(json_object_get(json_transaction,"timestamp")) < 0) return false;

    // 2: Sender
    if (!json_object_get(json_transaction, "sender")) return false;
    if (!json_is_string(json_object_get(json_transaction, "sender"))) return false;
    if (!sha256_hash_is_valid(json_string_value(json_object_get(json_transaction, "sender")))) return false;

    // 3: Receiver
    if (!json_object_get(json_transaction, "receiver")) return false;
    if (!json_is_string(json_object_get(json_transaction, "receiver"))) return false;
    if (!sha256_hash_is_valid(json_string_value(json_object_get(json_transaction, "receiver")))) return false;

    // 4: Amount
    if (!json_object_get(json_transaction, "amount")) return false;
    if (!json_is_real(json_object_get(json_transaction, "amount"))) return false;
    if (json_real_value(json_object_get(json_transaction, "amount")) < 0) return false;

    return true;
}

// Block

json_t *json_construct_block(const block_t *block) {
    json_t *json_block = json_object();
    json_t *json_block_transactions = json_array();

    if (json_object_set(json_block, "timestamp", json_integer(block->timestamp)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_block/json_object_set (timestamp)\n");
        return NULL;
    }

    if (json_object_set(json_block, "id", json_integer(block->id)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_block/json_object_set (id)\n");
        return NULL;
    }

    if (json_object_set(json_block, "nonce", json_integer(block->nonce)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_block/json_object_set (nonce)\n");
        return NULL;
    }

    if (json_object_set(json_block, "coinbase", json_real(block->coinbase)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_block/json_object_set (coinbase)\n");
        return NULL;
    }

    for (int i = 0; i < block->transaction_count; ++i) {
        if (json_array_append(json_block_transactions, json_construct_transaction(&block->transactions[i])) == -1) {
            fprintf(stderr, "\n[ERROR] json_construct_block/json_array_append (transactions)\n");
            return NULL;
        }
    }

    if (json_object_set(json_block, "transactions", json_block_transactions) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_block/json_object_set (transactions)\n");
        return NULL;
    }

    if (json_object_set(json_block, "previous", json_string(block->previous)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_block/json_object_set (previous)\n");
        return NULL;
    }

    if (json_object_set(json_block, "hash", json_string(block->hash)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_block/json_object_set (hash)\n");
        return NULL;
    }

    return json_block;
}

block_t json_destruct_block(const json_t *json_block) {
    block_t block;
    json_t *json_block_transactions = json_object_get(json_block, "transactions");

    block.timestamp = json_integer_value(json_object_get(json_block, "timestamp"));
    block.id = json_integer_value(json_object_get(json_block, "id"));
    block.nonce = json_integer_value(json_object_get(json_block, "nonce"));
    block.coinbase = json_real_value(json_object_get(json_block, "coinbase"));

    block.transaction_count = json_array_size(json_block_transactions);
    block.transactions = malloc(block.transaction_count * sizeof(transaction_t));
    for (int i = 0; i < block.transaction_count; ++i) block.transactions[i] = json_destruct_transaction(json_array_get(json_block_transactions, i));

    block.previous = json_string_value(json_object_get(json_block, "previous"));
    block.hash = json_string_value(json_object_get(json_block, "hash"));

    return block;
}

bool json_compare_blocks(const json_t *json_block1, const json_t *json_block2) {
    const block_t block1 = json_destruct_block(json_block1);
    const block_t block2 = json_destruct_block(json_block2);
    const json_t *block1_transactions = json_object_get(json_block1, "transactions");
    const json_t *block2_transactions = json_object_get(json_block2, "transactions");

    if (block1.timestamp != block2.timestamp) return false;
    if (block1.id != block2.id) return false;
    if (block1.nonce != block2.nonce) return false;
    if (block1.coinbase != block2.coinbase) return false;

    if (json_array_size(block1_transactions) != json_array_size(block2_transactions)) return false;
    for (int i = 0; i < json_array_size(block1_transactions); ++i) {
        if (!json_compare_transactions(json_array_get(block1_transactions, i), json_array_get(block2_transactions, i))) return false;
    }

    if (strcmp(block1.previous, block2.previous)) return false;
    if (strcmp(block1.hash, block2.hash)) return false;

    return true;
}

bool json_block_is_valid(const json_t *json_block) {
    if (!json_is_object(json_block)) return false;
    if (json_object_size(json_block) != 7) return false;

    // 1: Timestamp
    if (!json_object_get(json_block, "timestamp")) return false;
    if (!json_is_integer(json_object_get(json_block, "timestamp"))) return false;
    if (json_integer_value(json_object_get(json_block, "timestamp")) < 0) return false;

    // 2: ID
    if (!json_object_get(json_block, "id")) return false;
    if (!json_is_integer(json_object_get(json_block, "id"))) return false;
    if (json_integer_value(json_object_get(json_block, "id")) < 0) return false;

    // 3: Nonce
    if (!json_object_get(json_block, "nonce")) return false;
    if (!json_is_integer(json_object_get(json_block, "nonce"))) return false;
    if (json_integer_value(json_object_get(json_block, "nonce")) < 0) return false;

    // 4: Coinbase
    if (!json_object_get(json_block, "coinbase")) return false;
    if (!json_is_real(json_object_get(json_block, "coinbase"))) return false;
    if (json_real_value(json_object_get(json_block, "coinbase")) < 0) return false;

    // 5: Transactions
    if (!json_object_get(json_block, "transactions")) return false;
    if (!json_is_array(json_object_get(json_block, "transactions"))) return false;
    for (int i = 0; i < json_array_size(json_object_get(json_block, "transactions")); ++i) {
        if (!json_transaction_is_valid(json_array_get(json_object_get(json_block, "transactions"), i))) return false;
    }

    // 6: Previous
    if (!json_object_get(json_block, "previous")) return false;
    if (!json_is_string(json_object_get(json_block, "previous"))) return false;
    if (!sha256_hash_is_valid(json_string_value(json_object_get(json_block, "previous")))) return false;

    // 7: Hash
    if (!json_object_get(json_block, "hash")) return false;
    if (!json_is_string(json_object_get(json_block, "hash"))) return false;
    if (!sha256_hash_is_valid(json_string_value(json_object_get(json_block, "hash")))) return false;

    return true;
}

// Wallet

json_t *json_construct_wallet(const wallet_t *wallet) {
    json_t *json_wallet = json_object();

    if (json_object_set(json_wallet, "name", json_string(wallet->name)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_wallet/json_object_set (name)\n");
        return NULL;
    }

    if (json_object_set(json_wallet, "private_key", json_string(wallet->private_key)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_wallet/json_object_set (private key)\n");
        return NULL;
    }

    if (json_object_set(json_wallet, "public_key", json_string(wallet->public_key)) == -1) {
        fprintf(stderr, "\n[ERRROR] json_construct_wallet/json_object_set (public key)\n");
        return NULL;
    }

    if (json_object_set(json_wallet, "address", json_string(wallet->address)) == -1) {
        fprintf(stderr, "\n[ERROR] json_construct_wallet/json_object_set (address)\n");
        return NULL;
    }

    return json_wallet;
}

wallet_t json_destruct_wallet(const json_t *json_wallet) {
    wallet_t wallet;

    wallet.name = json_string_value(json_object_get(json_wallet, "name"));
    wallet.public_key = json_string_value(json_object_get(json_wallet, "private_key"));
    wallet.public_key = json_string_value(json_object_get(json_wallet, "public_key"));
    wallet.address = json_string_value(json_object_get(json_wallet, "address"));

    return wallet;
}

bool json_wallet_is_valid(const json_t *json_wallet) {
    if (!json_is_object(json_wallet)) return false;
    if (json_object_size(json_wallet) != 4) return false;

    // 1: Name
    if (!json_object_get(json_wallet, "name")) return false;
    if (!json_is_string(json_object_get(json_wallet, "name"))) return false;

    // 2: Private key
    if (!json_object_get(json_wallet, "private_key")) return false;
    if (!json_is_string(json_object_get(json_wallet, "private_key"))) return false;

    // 3: Public key
    if (!json_object_get(json_wallet, "public_key")) return false;
    if (!json_is_string(json_object_get(json_wallet, "public_key"))) return false;

    // 4: Address
    if (!json_object_get(json_wallet, "address")) return false;
    if (!json_is_string(json_object_get(json_wallet, "address"))) return false;

    return true;
}

// Extra

bool json_request_is_valid(const json_t *request_body, json_object_type_t object_type) {
    if (!json_is_object(request_body) && !json_is_array(request_body)) return false;

    if ((json_is_object(request_body) && !json_object_is_valid(request_body, object_type)) ||
        (json_is_array(request_body) && !json_array_is_valid(request_body, object_type))) return false;
    
    return true;
}

bool json_array_is_valid(const json_t *array, json_object_type_t object_type) {
    json_t *value;
    int index;

    if (!json_is_array(array)) return false;

    json_array_foreach(array, index, value) {
        if (!json_object_is_valid(value, object_type)) return false;
    }

    return true;
}

bool json_object_is_valid(const json_t *object, json_object_type_t object_type) {
    if (!json_is_object(object)) return false;

    switch (object_type) {
        case NODE: return json_node_is_valid(object);
        case TRANSACTION: return json_transaction_is_valid(object);
        case BLOCK: return json_block_is_valid(object);
        case WALLET: return json_wallet_is_valid(object);
        default: return false;
    }
}