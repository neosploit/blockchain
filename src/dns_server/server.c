#include <sys/stat.h>
#include <ulfius.h>
#include "argparse.h"
#include "server.h"
#include "auxilary.h"
#include "json.h"
#include "endpoints.h"

int retrieve_args(int argc, const char **argv, server_settings_t *server);
int confirm_settings(const server_settings_t *server);
int check_directory_files(const server_settings_t *server);
int check_file(const char *path, json_object_type_t object_type);
int update_nodes(const server_settings_t *server);

int main(int argc, const char **argv) {
    const char *executable_name = argv[0];
    int res;

    // Server
    server_settings_t server = { .port = 0, .request_timeout = 0, .update_interval = 0 };

    // Ulfius variables
    struct _u_instance instance;
    const struct _u_endpoint *endpoints[] = {
        endpoint_connection_check(),
        endpoint_node_retrieve(&server),
        endpoint_node_add(&server),
        endpoint_node_remove(&server),
        ulfius_empty_endpoint()
    };

    // Setup server settings
    if (retrieve_args(argc, argv, &server) == -1) {
        fprintf(stderr, "\n[ERROR] main/retrieve_args\n");
        return EXIT_FAILURE;
    }

    // Confirm server settings
    if (confirm_settings(&server) == -1) {
        fprintf(stdout, "\nType: %s -h, to get help\n", executable_name);
        return EXIT_SUCCESS;
    }

    // Check server's directory and files
    if (check_directory_files(&server) == -1) {
        fprintf(stderr, "\n[ERROR] main/check_directory_files\n");
        return EXIT_FAILURE;
    }

    // Initialize Ulfius instance
    if ((res = ulfius_init_instance(&instance, server.port, NULL, NULL)) != U_OK) {
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
        // Update nodes
        if (update_nodes(&server) == -1) {
            fprintf(stderr, "\n[ERROR] main/update_nodes\n");
            return EXIT_FAILURE;
        }

        sleep(server.update_interval);
    }

    // Stop framework and clean Ulfius instance
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return EXIT_SUCCESS;
}

int retrieve_args(int argc, const char **argv, server_settings_t *server) {
    // User input strings
    const char *directory = NULL;
    const char *nodes_filename = NULL;

    // argparse settings
    const char *usages[] = {
        "test_argparse [options] [[--] args]",
        "test_argparse [options]",
        NULL,
    };
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_INTEGER('p', "port", &server->port, "port to use", NULL, 0, 0),
        OPT_STRING('d', "dir", &directory, "server directory (e.g. ./.server/)", NULL, 0, 0),
        OPT_STRING('n', "nodes", &nodes_filename, "nodes filename (e.g. nodes.json)", NULL, 0, 0),
        OPT_INTEGER('t', "timeout", &server->request_timeout, "server request timeout in milliseconds (e.g. 100)", NULL, 0, 0),
        OPT_INTEGER('i', "interval", &server->update_interval, "server update interval in seconds (e.g. 60)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;

    // argparse
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    argc = argparse_parse(&argparse, argc, argv);

    // Set empty arguments to default values
    if (!server->port) server->port = SERVER_DEFAULT_PORT;
    (directory) ? asprintf(&server->directory, directory) : asprintf(&server->directory, SERVER_DEFAULT_DIRECTORY);
    (nodes_filename) ? asprintf(&server->nodes_path, "%s%s", server->directory, nodes_filename) : asprintf(&server->nodes_path, "%s%s", server->directory, SERVER_DEFAULT_NODES_FILENAME);
    if (!server->request_timeout) server->request_timeout = SERVER_DEFAULT_REQUEST_TIMEOUT;
    if (!server->update_interval) server->update_interval = SERVER_DEFAULT_UPDATE_INTERVAL;

    return 0;
}

int confirm_settings(const server_settings_t *server) {
    char user_input[3];

    // Ask user confirmation about used settings
    fprintf(stdout, "[INFO] The following server settings are going to be used:\n\n");
    fprintf(stdout, "Port: %hu\nNodes file path: %s\n", server->port, server->nodes_path);
    fprintf(stdout, "Server request timeout: %hu (milliseconds)\nServer update interval: %hu (seconds)\n\n", server->request_timeout, server->update_interval);
    fprintf(stdout, "Are you ok with that? (y/n): ");

    if (!fgets(user_input, sizeof(user_input), stdin)) return -1;
    if (user_input[0] != 'y') return -1;

    return 0;
}

int check_directory_files(const server_settings_t *server) {
    struct stat st; // Used by 'stat' function call

    // Check if server directory exists - create if not
    if (stat(server->directory, &st) == -1) {
        if (mkdir(server->directory, 0700) == -1) {
            perror("\n[ERROR] check_directory_files/mkdir (server directory)");
            return -1;
        }
    }

    // Check nodes file
    if (check_file(server->nodes_path, NODE) == -1) return -1;

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

int update_nodes(const server_settings_t *server) {
    json_t *json_loaded_nodes, *json_offline_nodes, *json_loaded_node;
    json_error_t json_error;
    int json_loaded_nodes_index, res;
    struct _u_request request;
    struct _u_response response;

    // Load all nodes from file
    if (!(json_loaded_nodes = json_load_file(server->nodes_path, 0, &json_error))) {
        fprintf(stderr, "\n[ERROR] update_nodes/json_load_file (%s): %s\n", server->nodes_path, json_error.text);
        return -1;
    }

    // Create a new empty array for offline nodes
    json_offline_nodes = json_array();

    // Iterate over all nodes, sending a request to each one's ENDPOINT_CONNECTION_CHECK_URL_PATH
    json_array_foreach(json_loaded_nodes, json_loaded_nodes_index, json_loaded_node) {
        node_t node = json_destruct_node(json_loaded_node);

        res = send_http_request(server, &request, &response, node.ip_address, node.port, ENDPOINT_CONNECTION_CHECK_HTTP_METHOD, ENDPOINT_CONNECTION_CHECK_URL_PATH, NULL);

        // If 'ulfius_send_http_request' function call fail with U_ERROR_LIBCURL, most probably the host was down. That's not an error.
        if (res != U_OK && res != U_ERROR_LIBCURL) {
            fprintf(stderr, "\n[ERROR] update_nodes/json_array_foreach/send_http_request (%s): %d\n", request.http_url, res);
            return -1;
        }

        // If request was not sent, add node to offline nodes
        if (res == U_ERROR_LIBCURL) json_array_append(json_offline_nodes, json_loaded_node);

        // Cleanup
        ulfius_clean_request(&request);
        ulfius_clean_response(&response);
    }

    // Finally, update nodes via server's own local endpoint
    res = send_http_request(server, &request, &response, SERVER_DEFAULT_IP_ADDRESS, server->port, ENDPOINT_NODE_REMOVE_HTTP_METHOD, ENDPOINT_NODE_REMOVE_URL_PATH, json_offline_nodes);

    if (res != U_OK) {
        fprintf(stderr, "\n[ERROR] update_nodes/send_http_request (%s): %d\n", request.http_url, res);
        return -1;
    }

    // Cleanup
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);

    return 0;
}