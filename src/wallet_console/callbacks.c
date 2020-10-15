#include <arpa/inet.h>
#include <ulfius.h>
#include "../client/client.h"
#include "../client/callbacks.h"
#include "../auxilary.h"
#include "../json.h"

// Connection

int callback_connection_check(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return U_CALLBACK_CONTINUE;
}

// Nodes

int callback_node_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes, *json_loaded_node;
    json_error_t json_error;
    char request_client_ip_address[INET_ADDRSTRLEN];
    int json_loaded_nodes_index, res;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_is_object(json_request_body) || !json_request_is_valid(json_request_body, NODE)) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUEST_JSON_DATA)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_node_retrieve/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUEST_JSON_DATA, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(inet_ntop(AF_INET, &(((struct sockaddr_in *) request->client_address)->sin_addr), request_client_ip_address, INET_ADDRSTRLEN))) {
        perror("\n[ERROR] callback_node_retrieve/inet_ntop");
        return U_CALLBACK_ERROR;
    }

    if (json_object_set(json_request_body, "ip_address", json_string(request_client_ip_address)) == -1) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_object_set (ip_address)\n");
        return U_CALLBACK_ERROR;
    }

    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        if (json_compare_nodes(json_request_body, json_loaded_node)) break;
    }

    if (json_loaded_nodes_index < json_array_size(json_loaded_nodes)) {
        if (json_array_remove(json_loaded_nodes, json_loaded_nodes_index) == -1) {
            fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_array_remove (%d)\n", json_loaded_nodes_index);
            return U_CALLBACK_ERROR;
        }
    }

    if ((res = ulfius_set_json_body_response(response, 200, json_loaded_nodes)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

int callback_node_add(const struct _u_request *request, struct _u_response *response, void *user_data) {
    client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes;
    json_error_t json_error;
    int res;

    if (client->known_nodes == client->known_nodes_limit) return U_CALLBACK_CONTINUE;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_add/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_request_is_valid(json_request_body, NODE)) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUEST_JSON_DATA)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_node_add/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUEST_JSON_DATA, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_add/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (json_is_object(json_request_body)) {
        json_t *json_loaded_node;
        int json_loaded_nodes_index;

        json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
            if (json_compare_nodes(json_request_body, json_loaded_node)) break;
        }

        if (json_loaded_nodes_index == json_array_size(json_loaded_nodes)) {
            if (json_array_append(json_loaded_nodes, json_request_body) == -1) {
                fprintf(stderr, "\n[ERROR] callback_node_add/json_is_object/json_array_append\n");
                return U_CALLBACK_ERROR;
            }

            client->known_nodes = json_array_size(json_loaded_nodes);
        }
    } else if (json_is_array(json_request_body)) {
        json_t *json_request_node, *json_loaded_node;
        int json_request_index, json_loaded_nodes_index;

        json_array_foreach(json_request_body, json_request_index, json_request_node) {
            if (client->known_nodes == client->known_nodes_limit) break;

            json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
                if (json_compare_nodes(json_request_node, json_loaded_node)) break;
            }

            if (json_loaded_nodes_index == json_array_size(json_loaded_nodes)) {
                if (json_array_append(json_loaded_nodes, json_request_node) == -1) {
                    fprintf(stderr, "\n[ERROR] callback_node_add/json_is_array/json_array_append\n");
                    return U_CALLBACK_ERROR;
                }

                client->known_nodes = json_array_size(json_loaded_nodes);
            }
        }
    }

    if (json_dump_file(json_loaded_nodes, client->nodes_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_node_add/json_dump_file (%s)\n", client->nodes_path);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

int callback_node_remove(const struct _u_request *request, struct _u_response *response, void *user_data) {
    client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes;
    json_error_t json_error;
    int res;

    if (client->known_nodes == 0) return U_CALLBACK_CONTINUE;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_remove/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_request_is_valid(json_request_body, NODE)) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUEST_JSON_DATA)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_node_remove/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUEST_JSON_DATA, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_remove/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (json_is_object(json_request_body)) {
        json_t *json_loaded_node;
        int json_loaded_nodes_index;

        json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
            if (json_compare_nodes(json_request_body, json_loaded_node)) break;
        }

        if (json_loaded_nodes_index < json_array_size(json_loaded_nodes)) {
            if (json_array_remove(json_loaded_nodes, json_loaded_nodes_index) == -1) {
                fprintf(stderr, "\n[ERROR] callback_node_remove/json_is_object/json_array_remove (%d)\n", json_loaded_nodes_index);
                return U_CALLBACK_ERROR;
            }

            client->known_nodes = json_array_size(json_loaded_nodes);
        }
    } else if (json_is_array(json_request_body)) {
        json_t *json_request_node, *json_loaded_node;
        int json_request_index, json_loaded_nodes_index;

        json_array_foreach(json_request_body, json_request_index, json_request_node) {
            if (client->known_nodes == 0) break;

            json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
                if (json_compare_nodes(json_request_node, json_loaded_node)) break;
            }

            if (json_loaded_nodes_index < json_array_size(json_loaded_nodes)) {
                if (json_array_remove(json_loaded_nodes, json_loaded_nodes_index) == -1) {
                    fprintf(stderr, "\n[ERROR] callback_node_remove/json_is_array/json_array_remove (%d)\n", json_loaded_nodes_index);
                    return U_CALLBACK_ERROR;
                }

                client->known_nodes = json_array_size(json_loaded_nodes);
            }
        }
    }

    if (json_dump_file(json_loaded_nodes, client->nodes_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_node_remove/json_dump_file (%s)\n", client->nodes_path);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

// Blockchain

int callback_blockchain_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_loaded_blockchain;
    json_error_t json_error;
    int res;

    if (!(json_loaded_blockchain = json_load_file(client->blockchain_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_blockchain_retrieve/json_load_file (%s): %s\n", client->blockchain_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if ((res = ulfius_set_json_body_response(response, 200, json_loaded_blockchain)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_blockchain_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

int callback_blockchain_update(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_blockchain, *json_loaded_blockchain_block;
    json_error_t json_error;
    int json_loaded_blockchain_index, res;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_blockchain_update/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_is_array(json_request_body) || !json_request_is_valid(json_request_body, BLOCK)) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUEST_JSON_DATA)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_blockchain_update/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUEST_JSON_DATA, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_loaded_blockchain = json_load_file(client->blockchain_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_blockchain_update/json_load_file (%s): %s\n", client->blockchain_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (json_array_size(json_loaded_blockchain) &&
        json_compare_blocks(json_array_get(json_request_body, json_array_size(json_request_body) - 1), json_array_get(json_loaded_blockchain, json_array_size(json_loaded_blockchain) - 1))) {
        return U_CALLBACK_COMPLETE;
    }

    json_array_foreach(json_loaded_blockchain, json_loaded_blockchain_index, json_loaded_blockchain_block);

    if (json_dump_file(json_request_body, client->blockchain_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_blockchain_update/json_dump_file (%s)\n", client->blockchain_path);
        return U_CALLBACK_ERROR;
    }

    response->shared_data = json_request_body;

    return U_CALLBACK_CONTINUE;
}

// Blocks

int callback_block_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    const char *requested_block_id_str = u_map_get(request->map_url, "bid");
    const int requested_block_id = atoi(requested_block_id_str);
    json_t *json_loaded_blockchain, *json_requested_block;
    json_error_t json_error;
    int res;

    if (requested_block_id <= 0) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUESTED_BLOCK_ID)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_block_retrieve/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUESTED_BLOCK_ID, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_loaded_blockchain = json_load_file(client->blockchain_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_block_retrieve/json_load_file (%s): %s\n", client->blockchain_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (requested_block_id > json_array_size(json_loaded_blockchain)) {
        if ((res = ulfius_set_string_body_response(response, 404, ERROR_REQUESTED_BLOCK_NOT_FOUND)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_block_retrieve/ulfius_set_string_body_response (%s): %d\n", ERROR_REQUESTED_BLOCK_NOT_FOUND, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_requested_block = json_array_get(json_loaded_blockchain, requested_block_id - 1))) {
        fprintf(stderr, "\n[ERROR] callback_block_retrieve/json_array_get (%d)\n", requested_block_id - 1);
        return U_CALLBACK_ERROR;
    }

    if ((res = ulfius_set_json_body_response(response, 200, json_requested_block)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_block_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    response->shared_data = json_requested_block;

    return U_CALLBACK_CONTINUE;
}

int callback_block_transactions_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const json_t *json_requested_block = response->shared_data;
    json_t *json_block_transactions = json_object_get(json_requested_block, "transactions");
    int res;

    if ((res = ulfius_set_json_body_response(response, 200, json_block_transactions)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_block_transactions_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    response->shared_data = json_block_transactions;

    return U_CALLBACK_CONTINUE;
}

int callback_block_transaction_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const json_t *json_block_transactions = response->shared_data;
    const char *requested_transaction_id_str = u_map_get(request->map_url, "tid");
    const int requested_transaction_id = atoi(requested_transaction_id_str);
    json_t *json_requested_transaction;
    int res;

    if (requested_transaction_id <= 0) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUESTED_TRANSACTION_ID)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_block_transaction_retrieve/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUESTED_TRANSACTION_ID, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (requested_transaction_id > json_array_size(json_block_transactions)) {
        if ((res = ulfius_set_string_body_response(response, 404, ERROR_REQUESTED_TRANSACTION_NOT_FOUND)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_block_transaction_retrieve/ulfius_set_string_body_response (%s): %d\n", ERROR_REQUESTED_TRANSACTION_NOT_FOUND, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_requested_transaction = json_array_get(json_block_transactions, requested_transaction_id - 1))) {
        fprintf(stderr, "\n[ERROR] callback_block_transaction_retrieve/json_array_get (%d)\n", requested_transaction_id - 1);
        return U_CALLBACK_ERROR;
    }

    if ((res = ulfius_set_json_body_response(response, 200, json_requested_transaction)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_block_transaction_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

// Mempool / Transactions

int callback_mempool_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_loaded_mempool;
    json_error_t json_error;
    int res;

    if (!(json_loaded_mempool = json_load_file(client->mempool_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_mempool_retrieve/json_load_file (%s): %s\n", client->mempool_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    if ((res = ulfius_set_json_body_response(response, 200, json_loaded_mempool)) != U_OK) {
        fprintf(stderr, "\n[ERROR] callback_mempool_retrieve/ulfius_set_json_body_response: %d\n", res);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

int callback_mempool_add(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_mempool, *json_loaded_mempool_transaction;
    json_error_t json_error;
    int json_loaded_mempool_index, res;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_mempool_add/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_is_object(json_request_body) || !json_request_is_valid(json_request_body, TRANSACTION)) {
        if ((res = ulfius_set_string_body_response(response, 400, ERROR_INVALID_REQUEST_JSON_DATA)) != U_OK) {
            fprintf(stderr, "\n[ERROR] callback_mempool_add/ulfius_set_string_body_response (%s): %d\n", ERROR_INVALID_REQUEST_JSON_DATA, res);
            return U_CALLBACK_ERROR;
        }

        return U_CALLBACK_COMPLETE;
    }

    if (!(json_loaded_mempool = json_load_file(client->mempool_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_mempool_add/json_load_file (%s): %s\n", client->mempool_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    json_array_foreach(json_loaded_mempool, json_loaded_mempool_index, json_loaded_mempool_transaction) {
        if (json_compare_transactions(json_request_body, json_loaded_mempool_transaction)) break;
    }

    if (json_loaded_mempool_index < json_array_size(json_loaded_mempool)) {
        return U_CALLBACK_COMPLETE;
    }

    if (json_array_append(json_loaded_mempool, json_request_body) == -1) {
        fprintf(stderr, "\n[ERROR] callback_mempool_add/json_array_append\n");
        return U_CALLBACK_ERROR;
    }

    if (json_dump_file(json_loaded_mempool, client->mempool_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_mempool_add/json_dump_file (%s)\n", client->mempool_path);
        return U_CALLBACK_ERROR;
    }

    response->shared_data = json_request_body;

    return U_CALLBACK_CONTINUE;
}

int callback_mempool_remove(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *json_blockchain = response->shared_data;
    json_t *json_block_transactions = json_object_get(json_array_get(json_blockchain, json_array_size(json_blockchain) - 1), "transactions");
    json_t *json_loaded_mempool, *json_block_transactions_transaction;
    json_error_t json_error;
    int json_block_transactions_index;

    if (!(json_loaded_mempool = json_load_file(client->mempool_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_mempool_remove/json_load_file (%s): %s\n", client->mempool_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    json_array_foreach(json_block_transactions, json_block_transactions_index, json_block_transactions_transaction) {
        json_t *json_loaded_mempool_transaction;
        int json_loaded_mempool_index;

        json_array_foreach(json_loaded_mempool, json_loaded_mempool_index, json_loaded_mempool_transaction) {
            if (json_compare_transactions(json_block_transactions_transaction, json_loaded_mempool_transaction)) break;
        }

        if (json_loaded_mempool_index < json_array_size(json_loaded_mempool)) {
            if (json_array_remove(json_loaded_mempool, json_loaded_mempool_index) == -1) {
                fprintf(stderr, "\n[ERROR] callback_mempool_remove/json_array_remove (%d)\n", json_loaded_mempool_index);
                return U_CALLBACK_ERROR;
            }
        }
    }

    if (json_dump_file(json_loaded_mempool, client->mempool_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_mempool_remove/json_dump_file (%s)\n", client->mempool_path);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

// Network

int callback_network_relay(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const client_settings_t *client = (client_settings_t *) user_data;
    json_t *shared_data = response->shared_data;
    json_t *json_loaded_nodes, *json_loaded_node;
    json_error_t json_error;
    int json_loaded_nodes_index, res;
    struct _u_request new_request;
    struct _u_response new_response;

    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_network_relay/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        const node_t node = json_destruct_node(json_loaded_node);

        res = send_http_request(&new_request, &new_response, node.ip_address, node.port, request->http_verb, request->http_url, shared_data, client->request_timeout);

        if (res != U_OK && res != U_ERROR_LIBCURL) {
            fprintf(stderr, "\n[ERROR] callback_network_relay/send_http_request (%s): %d\n", new_request.http_url, res);
            return U_CALLBACK_ERROR;
        }

        // Cleanup
        ulfius_clean_request(&new_request);
        ulfius_clean_response(&new_response);
    }

    return U_CALLBACK_CONTINUE;
}
