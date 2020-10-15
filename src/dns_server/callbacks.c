#include <arpa/inet.h>
#include <ulfius.h>
#include "callbacks.h"
#include "server.h"
#include "../json.h"

// Connection

int callback_connection_check(const struct _u_request *request, struct _u_response *response, void *user_data) {
    return U_CALLBACK_CONTINUE;
}

// Nodes

int callback_node_retrieve(const struct _u_request *request, struct _u_response *response, void *user_data) {
    const server_settings_t *server = (server_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes, *json_loaded_node;
    json_error_t json_error;
    char request_client_ip_address[INET_ADDRSTRLEN];
    int json_loaded_nodes_index, res;

    if (!(json_request_body = ulfius_get_json_body_request(request, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/ulfius_get_json_body_request: %s\n", json_error.text);
        return U_CALLBACK_ERROR;
    }

    if (!json_is_object(json_request_body) || !json_node_is_valid(json_request_body)) {
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

    if (!(json_loaded_nodes = json_load_file(server->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_load_file (%s): %s\n", server->nodes_path, json_error.text);
        return U_CALLBACK_ERROR;
    }

    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        if (json_compare_nodes(json_request_body, json_loaded_node)) break;
    }

    if (json_loaded_nodes_index == json_array_size(json_loaded_nodes)) {
        if (json_array_append(json_loaded_nodes, json_request_body) == -1) {
            fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_array_append\n");
            return U_CALLBACK_ERROR;
        }

        if (json_dump_file(json_loaded_nodes, server->nodes_path, 0) == -1) {
            fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_dump_file (%s)\n", server->nodes_path);
            return U_CALLBACK_ERROR;
        }
    }

    if (json_loaded_nodes_index == json_array_size(json_loaded_nodes)) {
        if (json_array_remove(json_loaded_nodes, json_array_size(json_loaded_nodes) - 1) == -1) {
            fprintf(stderr, "\n[ERROR] callback_node_retrieve/json_array_remove (%d)\n", (int)(json_array_size(json_loaded_nodes) - 1));
            return U_CALLBACK_ERROR;
        }
    } else {
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
    server_settings_t *server = (server_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes;
    json_error_t json_error;
    int res;

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

    if (!(json_loaded_nodes = json_load_file(server->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_add/json_load_file (%s): %s\n", server->nodes_path, json_error.text);
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
        }
    } else if (json_is_array(json_request_body)) {
        json_t *json_request_node, *json_loaded_node;
        int json_request_index, json_loaded_nodes_index;

        json_array_foreach(json_request_body, json_request_index, json_request_node) {
            json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
                if (json_compare_nodes(json_request_node, json_loaded_node)) break;
            }

            if (json_loaded_nodes_index == json_array_size(json_loaded_nodes)) {
                if (json_array_append(json_loaded_nodes, json_request_node) == -1) {
                    fprintf(stderr, "\n[ERROR] callback_node_add/json_is_array/json_array_append\n");
                    return U_CALLBACK_ERROR;
                }
            }
        }
    }

    if (json_dump_file(json_loaded_nodes, server->nodes_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_node_add/json_dump_file (%s)\n", server->nodes_path);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}

int callback_node_remove(const struct _u_request *request, struct _u_response *response, void *user_data) {
    server_settings_t *server = (server_settings_t *) user_data;
    json_t *json_request_body, *json_loaded_nodes;
    json_error_t json_error;
    int res;

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

    if (!(json_loaded_nodes = json_load_file(server->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] callback_node_remove/json_load_file (%s): %s\n", server->nodes_path, json_error.text);
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
        }
    } else if (json_is_array(json_request_body)) {
        json_t *json_request_node, *json_loaded_node;
        int json_request_index, json_loaded_nodes_index;

        json_array_foreach(json_request_body, json_request_index, json_request_node) {
            json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
                if (json_compare_nodes(json_request_node, json_loaded_node)) break;
            }

            if (json_loaded_nodes_index < json_array_size(json_loaded_nodes)) {
                if (json_array_remove(json_loaded_nodes, json_loaded_nodes_index) == -1) {
                    fprintf(stderr, "\n[ERROR] callback_node_remove/json_is_array/json_array_remove (%d)\n", json_loaded_nodes_index);
                    return U_CALLBACK_ERROR;
                }
            }
        }
    }

    if (json_dump_file(json_loaded_nodes, server->nodes_path, 0) == -1) {
        fprintf(stderr, "\n[ERROR] callback_node_remove/json_dump_file (%s)\n", server->nodes_path);
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_CONTINUE;
}