#ifndef MINER_H
#define MINER_H

#include <netinet/in.h>
#include <jansson.h>
#include "json.h"
#include "../include/client/endpoints.h"

#define DIFFICULTY 8    // number of hexadecimal zeros in front of hash

// Default settings
#define MINER_DEFAULT_IP_ADDRESS           "127.0.0.1"
#define MINER_PORT_MIN                     52020
#define MINER_PORT_MAX                     62020
#define MINER_DEFAULT_DIRECTORY            "./.miner/"
#define MINER_DEFAULT_NODES_FILENAME       "nodes.json"
#define MINING_POOL_DEFAULT_IP_ADDRESS     "127.0.0.1"
#define MINING_POOL_DEFAULT_PORT           45000
#define MINER_DEFAULT_REQUEST_TIMEOUT      100
#define MINER_CONSOLE_UPDATE_INTERVAL      5

// Terminal Colours
#define RED     "\x1B[31m"      /*  Failure  */
#define GRN     "\x1B[32m"      /*  Success  */
#define YEL     "\x1B[33m"      /*  Waiting  */
#define BLU     "\x1B[34m"      /*  Process  */
#define MAG     "\x1B[35m"      /*  Request  */
#define CYN     "\x1B[36m"      /*  Info     */
#define WHT     "\x1B[37m"      /*  Default  */
#define RESET   "\x1B[0m"

// Miner settings
typedef struct miner_settings_t {
    in_port_t port;
    char *directory;
    char *nodes_path;
    char *rewarding_address;
    char *mining_pool_ip_address;
    in_port_t mining_pool_port;
    unsigned short int request_timeout;
    json_t *json_node;
} miner_settings_t;

// hashing
char *calculate_transaction_hash(transaction_t transaction);
unsigned int **calculate_transaction_hashes(transaction_t *transactions, int transaction_count);
unsigned int *calculate_transaction_hashes_1D(transaction_t *transactions, int transaction_count);
char *calculate_block_hash(block_t block, unsigned int **transaction_hashes, unsigned int nonce);
bool hash_ok(char *hash, int difficulty);

// stats
time_t start_time;
int accepted;
int rejected;

void print_stats();

#endif // MINER_H
