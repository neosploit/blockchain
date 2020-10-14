#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

// Server default settings

#define SERVER_DEFAULT_IP_ADDRESS           "127.0.0.1"
#define SERVER_DEFAULT_PORT                 42020
#define SERVER_DEFAULT_DIRECTORY            "./.server/"
#define SERVER_DEFAULT_NODES_FILENAME       "nodes.json"
#define SERVER_DEFAULT_REQUEST_TIMEOUT      100
#define SERVER_DEFAULT_UPDATE_INTERVAL      60

// Server settings

typedef struct server_settings_t {
    in_port_t port;
    char *directory;
    char *nodes_path;
    unsigned short int request_timeout;
    unsigned short int update_interval;
} server_settings_t;

#endif // SERVER_H