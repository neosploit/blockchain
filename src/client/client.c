#include <sys/stat.h>
#include <ulfius.h>
#include "argparse.h"
#include "client.h"
#include "auxilary.h"
#include "json.h"
#include "endpoints.h"

int retrieve_args(int argc, const char **argv, client_settings_t *client);
int confirm_settings(const client_settings_t *client);
int check_directory_files(const client_settings_t *client);
int check_file(const char *path, json_object_type_t object_type);
int discover_nodes(const client_settings_t *client);
int check_previously_known_nodes(const client_settings_t *client);
int retrieve_known_nodes_connections(const client_settings_t *client);
int contact_dns_server(const client_settings_t *client);

int main(int argc, const char **argv) {
    const char *executable_name = argv[0];
    int res;

    // Client
    client_settings_t client = { .port = 0, .known_nodes = 0, .known_nodes_limit = 0, .request_timeout = 0, .update_interval = 0 };

    // Ulfius variables
    struct _u_instance instance;
    const struct _u_endpoint *endpoints[] = {
        endpoint_connection_check(),
        endpoint_node_retrieve(&client),
        endpoint_node_add(&client),
        endpoint_node_remove(&client),
        endpoint_blockchain_retrieve(&client),
        endpoint_blockchain_update(&client),
        endpoint_blockchain_update_mempool(&client),
        endpoint_blockchain_update_relay(&client),
        endpoint_block_retrieve(&client),
        endpoint_block_transactions_retrieve(),
        endpoint_block_transaction_retrieve(),
        endpoint_mempool_retrieve(&client),
        endpoint_mempool_add(&client),
        endpoint_mempool_add_relay(&client),
        ulfius_empty_endpoint()
    };

    // Setup client settings
    if (retrieve_args(argc, argv, &client) == -1) {
        fprintf(stderr, "\n[ERROR] main/retrieve_args\n");
        return EXIT_FAILURE;
    }

    // Confirm client settings
    if (confirm_settings(&client) == -1) {
        fprintf(stdout, "\nType: %s -h, to get help\n", executable_name);
        return EXIT_SUCCESS;
    }

    // Check client's directory and files
    if (check_directory_files(&client) == -1) {
        fprintf(stderr, "\n[ERROR] main/check_directory_files\n");
        return EXIT_FAILURE;
    }

    // Construct client's JSON node object
    node_t node = { .ip_address = CLIENT_DEFAULT_IP_ADDRESS, .port = client.port };
    client.json_node = json_construct_node(&node);

    // Initialize Ulfius instance
    if ((res = ulfius_init_instance(&instance, client.port, NULL, NULL)) != U_OK) {
        fprintf(stderr, "\n[ERROR] main/ulfius_init_instance: %d\n", res);
        return EXIT_FAILURE;
    }

    // Setup Ulfius endpoints
    if ((res = ulfius_add_endpoint_list(&instance, endpoints)) != U_OK) {
        fprintf(stderr, "\n[ERROR] main/ulfius_add_endpoint_list: %d\n", res);
        return EXIT_FAILURE;
    }

    // Start Ulfius framework
    if ((res = ulfius_start_framework(&instance)) != U_OK) {
        fprintf(stderr, "\n[ERROR] main/ulfius_start_framework: %d\n", res);
        return EXIT_FAILURE;
    }

    // Main loop
    while (1) {
        // Discover network nodes if needed
        if (discover_nodes(&client) == -1) {
            fprintf(stderr, "\n[ERROR] main/discover_nodes\n");
            return EXIT_FAILURE;
        }

        sleep(client.update_interval);
    }

    // Stop framework and clean Ulfius instance
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return EXIT_SUCCESS;
}

int retrieve_args(int argc, const char **argv, client_settings_t *client) {
    srand(time(NULL));  // rand() may be called later

    // User input strings
    const char *directory = NULL;
    const char *nodes_filename = NULL;
    const char *blockchain_filename = NULL;
    const char *mempool_filename = NULL;
    const char *wallets_filename = NULL;

    // argparse settings
    const char *usages[] = {
        "test_argparse [options] [[--] args]",
        "test_argparse [options]",
        NULL,
    };
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_INTEGER('p', "port", &client->port, "port to use", NULL, 0, 0),
        OPT_STRING('d', "dir", &directory, "client directory (e.g. ./.client/)", NULL, 0, 0),
        OPT_STRING('n', "nodes", &nodes_filename, "nodes filename (e.g. nodes.json)", NULL, 0, 0),
        OPT_STRING('b', "blockchain", &blockchain_filename, "blockchain filename (e.g. blockchain.json)", NULL, 0, 0),
        OPT_STRING('m', "mempool", &mempool_filename, "mempool filename (e.g. mempool.json)", NULL, 0, 0),
        OPT_STRING('w', "wallets", &wallets_filename, "wallets filename (e.g. wallets.json)", NULL, 0, 0),
        OPT_INTEGER('l', "limit", &client->known_nodes_limit, "known nodes limit (e.g. 10)", NULL, 0, 0),
        OPT_INTEGER('t', "timeout", &client->request_timeout, "client request timeout in milliseconds (e.g. 100)", NULL, 0, 0),
        OPT_INTEGER('i', "interval", &client->update_interval, "client update interval in seconds (e.g. 60)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;

    // argparse
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    argc = argparse_parse(&argparse, argc, argv);

    // Set empty arguments to default values
    if (!client->port) client->port = rand() % (CLIENT_PORT_MAX + 1 - CLIENT_PORT_MIN) + CLIENT_PORT_MIN;
    (directory) ? asprintf(&client->directory, directory) : asprintf(&client->directory, CLIENT_DEFAULT_DIRECTORY);
    (nodes_filename) ? asprintf(&client->nodes_path, "%s%s", client->directory, nodes_filename) : asprintf(&client->nodes_path, "%s%s", client->directory, CLIENT_DEFAULT_NODES_FILENAME);
    (blockchain_filename) ? asprintf(&client->blockchain_path, "%s%s", client->directory, blockchain_filename) : asprintf(&client->blockchain_path, "%s%s", client->directory, CLIENT_DEFAULT_BLOCKCHAIN_FILENAME);
    (mempool_filename) ? asprintf(&client->mempool_path, "%s%s", client->directory, mempool_filename) : asprintf(&client->mempool_path, "%s%s", client->directory, CLIENT_DEFAULT_MEMPOOL_FILENAME);
    (wallets_filename) ? asprintf(&client->wallets_path, "%s%s", client->directory, wallets_filename) : asprintf(&client->wallets_path, "%s%s", client->directory, CLIENT_DEFAULT_WALLETS_FILENAME);
    if (!client->known_nodes_limit) client->known_nodes_limit = CLIENT_DEFAULT_KNOWN_NODES_LIMIT;
    if (!client->request_timeout) client->request_timeout = CLIENT_DEFAULT_REQUEST_TIMEOUT;
    if (!client->update_interval) client->update_interval = CLIENT_DEFAULT_UPDATE_INTERVAL;

    return 0;
}

int confirm_settings(const client_settings_t *client) {
    char user_input[3];

    // Ask user confirmation about used settings
    fprintf(stdout, "[INFO] The following client settings are going to be used:\n\n");
    fprintf(stdout, "Port: %hu\nNodes file path: %s\nBlockchain file path: %s\nMempool file path: %s\nWallets file path: %s\n", client->port, client->nodes_path, client->blockchain_path, client->mempool_path, client->wallets_path);
    fprintf(stdout, "Known nodes limit: %hu\nClient request timeout: %hu (milliseconds)\nClient update interval: %hu (seconds)\n\n", client->known_nodes_limit, client->request_timeout, client->update_interval);
    fprintf(stdout, "Are you ok with that? (y/n): ");

    if (!fgets(user_input, sizeof(user_input), stdin)) return -1;
    if (user_input[0] != 'y') return -1;

    return 0;
}

int check_directory_files(const client_settings_t *client) {
    struct stat st; // Used by 'stat' function call

    // Check if client directory exists - create if not
    if (stat(client->directory, &st) == -1) {
        if (mkdir(client->directory, 0700) == -1) {
            perror("\n[ERROR] check_directory_files/mkdir (client directory)");
            return -1;
        }
    }

    // Check nodes file
    if (check_file(client->nodes_path, NODE) == -1) return -1;

    // Check blockchain file
    if (check_file(client->blockchain_path, BLOCK)  == -1) return -1;

    // Check mempool file
    if (check_file(client->mempool_path, TRANSACTION) == -1) return -1;

    // Check wallets file
    if (check_file(client->wallets_path, WALLET) == -1) return -1;

    return 0;
}

int check_file(const char *path, json_object_type_t object_type) {
    struct stat st; // Used by 'stat' function call

    if (stat(path, &st) == -1) {
        const json_t *json_empty_array = json_array();

        if (json_dump_file(json_empty_array, path, 0) == -1) {
            fprintf(stderr, "\n[ERROR] check_file/json_dump_file (%s)\n", path);
            return -1;
        }
    } else {
        json_t *file_content;
        json_error_t json_error;

        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "\n[ERROR] check_file/S_ISREG (%s)\n", path);
            return -1;
        }

        if (!(file_content = json_load_file(path, 0, &json_error))) {
            fprintf(stderr, "\n[ERROR] check_file/json_load_file (%s): %s\n", path, json_error.text);
            return -1;
        }

        if (!json_array_is_valid(file_content, object_type)) {
            fprintf(stderr, "\n[ERROR] check_file/json_array_is_valid (%s)\n", path);
            return -1;
        }
    }

    return 0;
}

int discover_nodes(const client_settings_t *client) {
    // 1st phase: Check previously known nodes
    if (check_previously_known_nodes(client) == -1) return -1;

    // 2nd phase: Retrieve known nodes' connections
    if (client->known_nodes < client->known_nodes_limit && retrieve_known_nodes_connections(client) == -1) return -1;

    // 3rd phase: Contact DNS server
    if (client->known_nodes < client->known_nodes_limit && contact_dns_server(client) == -1) return -1;

    return 0;
}

int check_previously_known_nodes(const client_settings_t *client) {
    json_t *json_loaded_nodes, *json_offline_nodes, *json_loaded_node;
    json_error_t json_error;
    int json_loaded_nodes_index, res;
    struct _u_request request;
    struct _u_response response;

    // Load all nodes from file
    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] check_previously_known_nodes/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return -1;
    }

    // Create a new empty array for offline nodes
    json_offline_nodes = json_array();

    // Iterate over all nodes, sending a request to each one's ENDPOINT_CONNECTION_CHECK_URL_PATH
    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        node_t node = json_destruct_node(json_loaded_node);

        res = send_http_request(client, &request, &response, node.ip_address, node.port, ENDPOINT_CONNECTION_CHECK_HTTP_METHOD, ENDPOINT_CONNECTION_CHECK_URL_PATH, NULL);

        // If 'ulfius_send_http_request' function call fail with U_ERROR_LIBCURL, most probably the host was down. That's not an error.
        if (res != U_OK && res != U_ERROR_LIBCURL) {
            fprintf(stderr, "\n[ERROR] check_previously_known_nodes/json_array_foreach/send_http_request (%s): %d\n", request.http_url, res);
            return -1;
        }

        // If request was not sent, add node to offline nodes
        if (res == U_ERROR_LIBCURL) json_array_append(json_offline_nodes, json_loaded_node);

        // Cleanup
        ulfius_clean_request(&request);
        ulfius_clean_response(&response);
    }

    // Finally, update nodes via client's own local endpoint
    res = send_http_request(client, &request, &response, CLIENT_DEFAULT_IP_ADDRESS, client->port, ENDPOINT_NODE_REMOVE_HTTP_METHOD, ENDPOINT_NODE_REMOVE_URL_PATH, json_offline_nodes);

    if (res != U_OK) {
        fprintf(stderr, "\n[ERROR] check_previously_known_nodes/send_http_request (%s): %d\n", request.http_url, res);
        return -1;
    }

    // Cleanup
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);

    return 0;
}

int retrieve_known_nodes_connections(const client_settings_t *client) {
    json_t *json_loaded_nodes, *json_loaded_node;
    json_error_t json_error;
    int json_loaded_nodes_index, res;
    struct _u_request request;
    struct _u_response response;

    // Load all nodes from file
    if (!(json_loaded_nodes = json_load_file(client->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] retrieve_known_nodes_connections/json_load_file (%s): %s\n", client->nodes_path, json_error.text);
        return -1;
    }

    // Iterate over all nodes, sending a request to each one's ENDPOINT_NODE_RETRIEVE_URL_PATH
    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        node_t node = json_destruct_node(json_loaded_node);

        res = send_http_request(client, &request, &response, node.ip_address, node.port, ENDPOINT_NODE_RETRIEVE_HTTP_METHOD, ENDPOINT_NODE_RETRIEVE_URL_PATH, client->json_node);

        // If 'ulfius_send_http_request' function call fail with U_ERROR_LIBCURL, most probably the host was down. That's not an error.
        if (res != U_OK && res != U_ERROR_LIBCURL) {
            fprintf(stderr, "\n[ERROR] retrieve_known_nodes_connections/json_array_foreach/send_http_request (%s): %d\n", request.http_url, res);
            return -1;
        }

        // If the response is valid, retrieve the JSON node array from response's body
        if (res == U_OK && response.status == 200) {
            json_t *response_nodes;

            if (!(response_nodes = ulfius_get_json_body_response(&response, &json_error))) {
                fprintf(stderr, "\n[ERROR] retrieve_known_nodes_connections/json_array_foreach/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
                return -1;
            }

            // Cleanup for the next request
            ulfius_clean_request(&request);
            ulfius_clean_response(&response);

            // Notify DNS server about a new node
            res = send_http_request(client, &request, &response, CLIENT_DEFAULT_IP_ADDRESS, client->port, ENDPOINT_NODE_ADD_HTTP_METHOD, ENDPOINT_NODE_ADD_URL_PATH, response_nodes);

            if (res != U_OK) {
                fprintf(stderr, "\n[ERROR] retrieve_known_nodes_connections/json_array_foreach/send_http_request (%s): %d\n", request.http_url, res);
                return -1;
            }
        }

        // Cleanup
        ulfius_clean_request(&request);
        ulfius_clean_response(&response);
    }

    return 0;
}

int contact_dns_server(const client_settings_t *client) {
    json_t *json_response_nodes;
    json_error_t json_error;
    int res;
    struct _u_request request;
    struct _u_response response;

    res = send_http_request(client, &request, &response, PRIMARY_DNS_SERVER_IP_ADDRESS, PRIMARY_DNS_SERVER_PORT, ENDPOINT_NODE_RETRIEVE_HTTP_METHOD, ENDPOINT_NODE_RETRIEVE_URL_PATH, client->json_node);

    // If 'ulfius_send_http_request' function call fail with U_ERROR_LIBCURL, most probably the host was down. That's not an error.
    if (res != U_OK && res != U_ERROR_LIBCURL) {
        fprintf(stderr, "\n[ERROR] contact_dns_server/send_http_request (%s): %d\n", request.http_url, res);
        return -1;
    }

    // If the response is valid, retrieve the JSON node array from response's body
    if (res == U_OK && response.status == 200) {
        if (!(json_response_nodes = ulfius_get_json_body_response(&response, &json_error))) {
            fprintf(stderr, "\n[ERROR] contact_dns_server/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
            return -1;
        }

        // Cleanup
        ulfius_clean_request(&request);
        ulfius_clean_response(&response);

        // Update nodes via client's own local endpoint
        res = send_http_request(client, &request, &response, CLIENT_DEFAULT_IP_ADDRESS, client->port, ENDPOINT_NODE_ADD_HTTP_METHOD, ENDPOINT_NODE_ADD_URL_PATH, json_response_nodes);

        if (res != U_OK) {
            fprintf(stderr, "\n[ERROR] contact_dns_server/send_http_request (%s): %d\n", request.http_url, res);
            return -1;
        }
    }

    // Cleanup
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);

    return 0;
}