#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <jansson.h>

// Client default settings

#define CLIENT_DEFAULT_IP_ADDRESS           "127.0.0.1"
#define CLIENT_PORT_MIN                     52020
#define CLIENT_PORT_MAX                     62020
#define CLIENT_DEFAULT_DIRECTORY            "./.wallet_gui/"
#define CLIENT_DEFAULT_NODES_FILENAME       "nodes.json"
#define CLIENT_DEFAULT_BLOCKCHAIN_FILENAME  "blockchain.json"
#define CLIENT_DEFAULT_MEMPOOL_FILENAME     "mempool.json"
#define CLIENT_DEFAULT_WALLETS_FILENAME     "wallets.json"
#define CLIENT_DEFAULT_KNOWN_NODES_LIMIT    10
#define CLIENT_DEFAULT_REQUEST_TIMEOUT      100
#define CLIENT_DEFAULT_UPDATE_INTERVAL      60

// DNS server info

#define PRIMARY_DNS_SERVER_IP_ADDRESS       "127.0.0.1"
#define PRIMARY_DNS_SERVER_PORT             42020

// Client settings

typedef struct client_settings_t {
    in_port_t port;
    char *directory;
    char *nodes_path;
    char *blockchain_path;
    char *mempool_path;
    char *wallets_path;
    unsigned short int known_nodes;
    unsigned short int known_nodes_limit;
    unsigned short int request_timeout;
    unsigned short int update_interval;
    json_t *json_node;
} client_settings_t;

#endif // CLIENT_H