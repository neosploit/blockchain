#include <arpa/inet.h>
#include <sys/stat.h>
#include <jansson.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <ulfius.h>
#include "../../include/miner.h"
#include "../../include/argparse.h"
#include "../../include/auxilary.h"
#include "../../include/json.h"
#include "../../include/hashing.h"

// settings
int retrieve_args(int argc, const char **argv, miner_settings_t *miner);
int confirm_settings(const miner_settings_t *miner);

// check files
int check_directory_files(const miner_settings_t *miner);
int check_file(const char *path, json_object_type_t object_type);

// check mining pool
int mining_pool_up(const miner_settings_t *miner);

/* hashing functions are declared in header */

// retrieve functions
int get_last_block(miner_settings_t *miner, block_t *last_block, int *block_index);
int get_mempool_transactions(miner_settings_t *miner, transaction_t **transactions, int *transaction_count);

// block mining
int create_new_block(miner_settings_t *miner, block_t *block);
int brute_force_solve_block(block_t *block);
int send_block_solution(miner_settings_t *miner, block_t *block);
int send_reward_transaction(miner_settings_t *miner, block_t *block);

int main(int argc, const char **argv){
    const char *executable_name = argv[0];

    miner_settings_t miner = { .request_timeout = 0 };

    // Setup miner settings
    if (retrieve_args(argc, argv, &miner) == -1) {
        fprintf(stderr, "\n[ERROR] main/retrieve_args\n");
        return EXIT_FAILURE;
    }

    // Confirm miner settings
    if (confirm_settings(&miner) == -1) {
        fprintf(stdout, "\nType: %s -h, to get help\n", executable_name);
        return EXIT_SUCCESS;
    }

    // Check miner's directory and files
    if (check_directory_files(&miner) == -1) {
        fprintf(stderr, "\n[ERROR] main/check_directory_files\n");
        return EXIT_FAILURE;
    }

    // Construct miner's JSON node object
    node_t node = {.ip_address = MINER_DEFAULT_IP_ADDRESS, .port = miner.port};
    miner.json_node = json_construct_node(&node);

    // check if mining pool is up
    if(mining_pool_up(&miner)){
        printf(RED "\nMining Pool %s:%hu is down!\n" RESET, miner.mining_pool_ip_address, miner.mining_pool_port);
        return EXIT_FAILURE;
    }
    else{
        printf(GRN "\nSuccessfully Connected to Mining Pool %s:%hu!\n" RESET, miner.mining_pool_ip_address, miner.mining_pool_port);
    }

    // miner variables
    block_t block;
    transaction_t *transactions;
    int transaction_count;

    // stats
    time(&start_time);
    accepted = 0;
    rejected = 0;
    
    // main loop
    while(1){
        // wait for uncomfirmed transactions
        do{
            // get uncomfirmed mempool transactions
            if(get_mempool_transactions(&miner, &transactions, &transaction_count)){
                fprintf(stderr, "\n[ERROR] main/get_mempool_transactions\n");
                return EXIT_FAILURE;
            }

            if(transaction_count == 0){
                printf(YEL "\nWaiting for at least 1 unconfirmed transaction!\n" RESET);
                sleep(MINER_CONSOLE_UPDATE_INTERVAL);
            }
            else{
                printf(WHT "\n%d Transaction(s) can now be combined into a block!\n" RESET, transaction_count);
            }
        } while(transaction_count == 0);

        // solve block if there are unconfirmed transactions
        if(create_new_block(&miner, &block)){
            fprintf(stderr, "\n[ERROR] main/create_new_block\n");
            return EXIT_FAILURE;
        }
        if(brute_force_solve_block(&block) == EXIT_SUCCESS){
            if(send_block_solution(&miner, &block) == EXIT_SUCCESS){
                accepted++;
                send_reward_transaction(&miner, &block);
            }
            else{
                rejected++;
            }
        }

        // cleanup
        free(transactions);
    }

    return EXIT_SUCCESS;
}

// settings
int retrieve_args(int argc, const char **argv, miner_settings_t *miner){
    const char *directory = NULL;
    const char *nodes_filename = NULL;
    const char *address = NULL;
    const char *mp_ip_address = NULL;
    
    // argparse settings
     const char *usages[] = {
        "test_argparse [options] [[--] args]",
        "test_argparse [options]",
        NULL,
    };
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_INTEGER('p', "port", &miner->port, "port to use", NULL, 0, 0),
        OPT_STRING('d', "dir", &directory, "miner directory (e.g. ./.miner/)", NULL, 0, 0),
        OPT_STRING('n', "nodes", &nodes_filename, "nodes filename (e.g. nodes.json)", NULL, 0, 0),
        OPT_STRING('a', "address", &address, "miner rewarding address as hexaxedimal", NULL, 0, 0),
        OPT_STRING('i', "miningpool_ip", &mp_ip_address, "mining pool ip address", NULL, 0, 0),
        OPT_INTEGER('m', "miningpool_port", &miner->mining_pool_port, "mining pool port", NULL, 0, 0),
        OPT_INTEGER('t', "timeout", &miner->request_timeout, "miner request timeout in milliseconds (e.g. 100)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;

    // argparse
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    argc = argparse_parse(&argparse, argc, argv);

    // Set empty arguments to default values
    if (!miner->port) miner->port = rand() % (MINER_PORT_MAX + 1 - MINER_PORT_MIN) + MINER_PORT_MIN;
    (directory) ? asprintf(&miner->directory, directory) : asprintf(&miner->directory, MINER_DEFAULT_DIRECTORY);
    (nodes_filename) ? asprintf(&miner->nodes_path, "%s%s", miner->directory, nodes_filename) : asprintf(&miner->nodes_path, "%s%s", miner->directory, MINER_DEFAULT_NODES_FILENAME);
    (address) ? asprintf(&miner->rewarding_address, "%s", address) : asprintf(&miner->rewarding_address, "%0*x", HEX_LENGTH, 0);
    (mp_ip_address) ? asprintf(&miner->mining_pool_ip_address, "%s", mp_ip_address) : asprintf(&miner->mining_pool_ip_address, "%s", MINING_POOL_DEFAULT_IP_ADDRESS);
    if (!miner->mining_pool_port) miner->mining_pool_port = MINING_POOL_DEFAULT_PORT;
    if (!miner->request_timeout) miner->request_timeout = MINER_DEFAULT_REQUEST_TIMEOUT;

    return 0;
}

int confirm_settings(const miner_settings_t *miner) {
    char user_input[3];

    // Ask user confirmation about used settings
    fprintf(stdout, "[INFO] The following miner settings are going to be used:\n\n");
    fprintf(stdout, "Port: %hu\nNodes file path: %s\nRewarding Address: %s\n", miner->port, miner->nodes_path, miner->rewarding_address);
    fprintf(stdout, "Mining Pool: %s:%hu\n", miner->mining_pool_ip_address, miner->mining_pool_port);
    fprintf(stdout, "Miner request timeout: %hu (milliseconds)\n\n", miner->request_timeout);
    fprintf(stdout, "Are you ok with that? (y/n): ");

    if (!fgets(user_input, sizeof(user_input), stdin)) return -1;
    if (user_input[0] != 'y') return -1;

    return 0;
}

// check files
int check_directory_files(const miner_settings_t *miner) {
    struct stat st; // Used by 'stat' function call

    // Check if miner directory exists - create if not
    if (stat(miner->directory, &st) == -1) {
        if (mkdir(miner->directory, 0700) == -1) {
            perror("\n[ERROR] check_directory_files/mkdir (miner directory)");
            return -1;
        }
    }

    // Check nodes file
    if (check_file(miner->nodes_path, NODE) == -1) return -1;

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

// check mining pool
int mining_pool_up(const miner_settings_t *miner){
    int res;
    struct _u_request request;
    struct _u_response response;
    
    // send http request
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_NODE_RETRIEVE_HTTP_METHOD, ENDPOINT_NODE_RETRIEVE_URL_PATH, miner->json_node, miner->request_timeout);

    // check if host down
    if (res != U_OK && res != U_ERROR_LIBCURL) {
        return EXIT_FAILURE;
    }
    
    if(res == U_OK && response.status == 200){
        return EXIT_SUCCESS;
    }
    
    return EXIT_FAILURE;
}

// hashing
char *calculate_transaction_hash(transaction_t transaction){
    char *hash_string;
    unsigned int *hash;
    unsigned int *sender;
    unsigned int *receiver;
    int i;

    // convert sender and receiver into int arrays
    sender = hash_to_int_array(transaction.sender);
    receiver = hash_to_int_array(transaction.receiver);

    // allocate memory
    hash = (unsigned int*) calloc((int)(HEX_LENGTH / HEX_GROUP), sizeof(unsigned int));

    // calculate hash
    hash[0] = (unsigned int) (transaction.timestamp * transaction.amount);
    hash[0] = hash[0] ^ (sender[0] | receiver[0]);
    for(i = 1; i < (int)(HEX_LENGTH / HEX_GROUP); i++){
        hash[i] = hash[i-1] ^ (sender[i] | receiver[i]);
    }

    // store as character array
    hash_string = int_array_to_hash(hash);

    // free memory
    free(hash);
    free(sender);
    free(receiver);

    return hash_string;
}

unsigned int **calculate_transaction_hashes(transaction_t *transactions, int transaction_count){
    char *transaction_hash_string;
    unsigned int **transaction_hashes;
    int i;

    transaction_hashes = (unsigned int **) malloc(transaction_count * sizeof(unsigned int*));
    for (i = 0; i < transaction_count; i++){
        transaction_hash_string = calculate_transaction_hash(transactions[i]);
        transaction_hashes[i] = hash_to_int_array(transaction_hash_string);
        free(transaction_hash_string);
    }
    
    return transaction_hashes;
}

unsigned int *calculate_transaction_hashes_1D(transaction_t *transactions, int transaction_count){
    char *transaction_hash_string;
    unsigned int *transaction_hash;
    unsigned int *transaction_hashes;
    int i, j;
    transaction_hashes = (unsigned int *) malloc(transaction_count * (int)(HEX_LENGTH / HEX_GROUP) * sizeof(unsigned int));

    for(i = 0; i < transaction_count; i++){
        // calculate transaction hash
        transaction_hash_string = calculate_transaction_hash(transactions[i]);
        transaction_hash = hash_to_int_array(transaction_hash_string);
        // store in 1D array
        for(j = 0; j < (int)(HEX_LENGTH / HEX_GROUP); j++){
            transaction_hashes[i * (int)(HEX_LENGTH / HEX_GROUP) + j] = transaction_hash[j];
        }

        free(transaction_hash);
        free(transaction_hash_string);
    }

    return transaction_hashes;
}

char *calculate_block_hash(block_t block, unsigned int **transaction_hashes, unsigned int nonce){
    char *hash_string;
    unsigned int *hash;
    int i, j;
    unsigned int *previous_hash;
    unsigned int words[(HEX_LENGTH / HEX_GROUP) * 8];
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int m, k, temp; 

    // allocate memory
    hash = (unsigned int*) calloc(HEX_LENGTH / HEX_GROUP, sizeof(unsigned int));
    
    /* Init Hash with Previous Block's Hash */
    previous_hash = hash_to_int_array(block.previous);
    for (i = 0; i < (int)(HEX_LENGTH / HEX_GROUP); i++){
        hash[i] = previous_hash[i];
    }

    /* Step 2 - Add Block Info */
    words[0] = nonce;
    words[1] = (block.timestamp >> 32) % UINT_MAX;
    words[2] = block.timestamp % UINT_MAX;
    words[3] = block.id;

    for (i = 4; i < (HEX_LENGTH / HEX_GROUP) * 8; i++){
        words[i] = ((words[i - 2] ^ words[i - 4]) << 3) | ((words[i - 2] ^ words[i - 4]) >> 29);
    }

    /* Main Loop */
    for (i = 0; i < (HEX_LENGTH / HEX_GROUP) * 8; i++){
        a = hash[0];
        b = hash[1];
        c = hash[2];
        d = hash[3];
        e = hash[4];
        f = hash[5];
        g = hash[6];
        h = hash[7];
    
        if (i < 16){
            m = (b & c) | ((~ b) & d) | (b & (~e)) | ((~b) & (~f)) | (b & (~g)) | ((~b) & h);
            k = 1518500249; // 0x5A827999
        }
        else if (i < 32){
            m = b ^ c ^ d ^ e ^ f ^ g ^ h;
            k = 1859775393; // 0x6ED9EBA1
        }
        else if (i < 48){
            m = (b & c) | (b & d) | (b & e) | (b & f) | (b & g) | (b & h);
            k = 2400959708; // 0x8F1BBCDC
        }
        else{
            m = b ^ c ^ d ^ e ^ f ^ g ^ h;
            k = 3395469782; // 0xCA62C1D6
        }
        
        temp = ((a << 5) | (a >> 27)) ^ (m & k) ^ words[i];

        h = g;
        g = (f << 29) | (f >> 3);
        f = e;
        e = d;
        d = c;
        c = (b << 13) | (b >> 19);
        b = a;
        a = temp;

        hash[0] ^= a;
        hash[1] ^= b;
        hash[2] ^= c;
        hash[3] ^= d;
        hash[4] ^= e;
        hash[5] ^= f;
        hash[6] ^= g;
        hash[7] ^= h;
    }

    /* Step 3 - Add Transaction Hashes */
    for (i = 0; i < block.transaction_count; i++){
        for (j = 0; j < (HEX_LENGTH / HEX_GROUP); j++){
            words[j] = transaction_hashes[i][j];
        }
        
        for (j = (HEX_LENGTH / HEX_GROUP); j < (HEX_LENGTH / HEX_GROUP) * 8; j++){
            words[j] = ((words[j - 2] ^ words[j - 4]) << 3) | ((words[j - 2] ^ words[j - 4]) >> 29);
        }

        /* Main Loop */
        for (j = 0; j < (HEX_LENGTH / HEX_GROUP) * 8; j++){
            a = hash[0];
            b = hash[1];
            c = hash[2];
            d = hash[3];
            e = hash[4];
            f = hash[5];
            g = hash[6];
            h = hash[7];

            if (j < 16){
                m = (b & c) | ((~ b) & d) | (b & (~e)) | ((~b) & (~f)) | (b & (~g)) | ((~b) & h);
                k = 1518500249; // 0x5A827999
            }
            else if (j < 32){
                m = b ^ c ^ d ^ e ^ f ^ g ^ h;
                k = 1859775393; // 0x6ED9EBA1
            }
            else if (j < 48){
                m = (b & c) | (b & d) | (b & e) | (b & f) | (b & g) | (b & h);
                k = 2400959708; // 0x8F1BBCDC
            }
            else{
                m = b ^ c ^ d ^ e ^ f ^ g ^ h;
                k = 3395469782; // 0xCA62C1D6
            }

            temp = ((a << 5) | (a >> 27)) ^ (m & k) ^ words[i];

            h = g;
            g = (f << 29) | (f >> 3);
            f = e;
            e = d;
            d = c;
            c = (b << 13) | (b >> 19);
            b = a;
            a = temp;

            hash[0] ^= a;
            hash[1] ^= b;
            hash[2] ^= c;
            hash[3] ^= d;
            hash[4] ^= e;
            hash[5] ^= f;
            hash[6] ^= g;
            hash[7] ^= h;
        }
    }

    // store as character array
    hash_string = int_array_to_hash(hash);

    // free memory
    free(hash);
    free(previous_hash);

    return hash_string;
}

bool hash_ok(char *hash, int difficulty){
    int i;
    int zero_count = 0;

    for(i = 0; i < HEX_LENGTH; i++){
        if(hash[i] == '0'){
            zero_count++;
        }
        else{
            break;
        }
    }

    if(zero_count == difficulty){
        return true;
    }
    else{
        return false;
    }
}

// retrieve functions
int get_last_block(miner_settings_t *miner, block_t *last_block, int *block_index){
    json_error_t json_error;
    int res;
    struct _u_request request;
    struct _u_response response;
    json_t *blockchain;
    block_t block;
    
    // send http request
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH, NULL, miner->request_timeout);

    // check if host down
    if (res != U_OK && res != U_ERROR_LIBCURL) {
        fprintf(stderr, "\n[ERROR] get_last_block/send_http_request (%s): %d\n", request.http_url, res);
        return EXIT_FAILURE;
    }

    // retrieve blockchain from response
    if(res == U_OK && response.status == 200){
        if (!(blockchain = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] get_last_block/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
            return EXIT_FAILURE;
        }
    }

    // find last block
    *block_index = json_array_size(blockchain) - 1;

    if((*block_index) >= 0){
        block = json_destruct_block(json_array_get(blockchain, *block_index));
    }
    
    // cleanup
    free(blockchain);
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);

    *last_block = block;

    return EXIT_SUCCESS;
}

int get_mempool_transactions(miner_settings_t *miner, transaction_t **transactions, int *transaction_count){
    json_error_t json_error;
    int res;
    struct _u_request request;
    struct _u_response response;
    json_t *mempool;
    int i;
    
    // send http request
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_MEMPOOL_RETRIEVE_HTTP_METHOD, ENDPOINT_MEMPOOL_RETRIEVE_URL_PATH, NULL, miner->request_timeout);

    // check if host down
    if (res != U_OK && res != U_ERROR_LIBCURL) {
        fprintf(stderr, "\n[ERROR] get_last_block/send_http_request (%s): %d\n", request.http_url, res);
        return EXIT_FAILURE;
    }

    // retrieve mempool array from response
    if(res == U_OK && response.status == 200){
        if (!(mempool = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] get_last_block/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
            return EXIT_FAILURE;
        }
    }

    *transaction_count = json_array_size(mempool);

    if(*transaction_count > 0){
        transactions[0] = (transaction_t*) malloc((*transaction_count) * sizeof(transaction_t));
        for(i = 0; i < (*transaction_count); i++){
            transactions[0][i] = json_destruct_transaction(json_array_get(mempool, i));
        }
    }

    // cleanup
    free(mempool);
    ulfius_clean_request(&request);
    ulfius_clean_response(&response);

    return EXIT_SUCCESS;
}

// main functions
int create_new_block(miner_settings_t *miner, block_t *block){
    block_t last_block;
    int last_block_index;
    transaction_t *transactions;
    int transaction_count;
    char *genesis_hash;
    int i;

    // get last block of the blockchain
    if(get_last_block(miner, &last_block, &last_block_index)){
        fprintf(stderr, "\n[ERROR] create_new_block/get_last_block\n");
        return EXIT_FAILURE;
    }

    // get unconfirmed transactions from the mempool
    if(get_mempool_transactions(miner, &transactions, &transaction_count)){
        fprintf(stderr, "\n[ERROR] create_new_block/get_mempool_transactions\n");
        return EXIT_FAILURE;
    }

    block->timestamp = (unsigned long int) time(NULL);
    if(last_block_index >= 0){
        block->id = last_block.id + 1;
    }
    else{
        block->id = 0;
    }
    block->nonce = 0;
    block->coinbase = 12.5;
    block->transaction_count = transaction_count;
    block->transactions = transactions;
    if(last_block_index >= 0){
        block->previous = last_block.hash;
    }
    else{
        genesis_hash = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));
        for(i = 0; i < HEX_LENGTH; i++){
            genesis_hash[i] = '0';
        }
        genesis_hash[HEX_LENGTH] = '\0';
        block->previous = genesis_hash;
    }

    return EXIT_SUCCESS;
}

int send_block_solution(miner_settings_t *miner, block_t *block){
    json_t *json_blockchain;
    json_t *json_block;
    json_error_t json_error;
    int res;
    struct _u_request request;
    struct _u_response response;

    // construct block
    json_block = json_construct_block(block);

    json_dump_file(json_block, "test_blockchain.json", JSON_ENCODE_ANY);

    // send http request
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH, NULL, miner->request_timeout);

    // check if host down
    if (res != U_OK && res != U_ERROR_LIBCURL) {
        fprintf(stderr, "\n[ERROR] get_last_block/send_http_request (%s): %d\n", request.http_url, res);
        return EXIT_FAILURE;
    }
    
    // retrieve blockchain from response
    if(res == U_OK && response.status == 200){
        if (!(json_blockchain = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] get_last_block/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
            return EXIT_FAILURE;
        }
    }

    // add new block to blockchain
    json_array_append(json_blockchain, json_block);

    // send solution to node
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_BLOCKCHAIN_UPDATE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_UPDATE_URL_PATH, json_blockchain, miner->request_timeout);        

    if(res != U_OK){
        printf(RED "Mining Pool %s:%hu is down!\n" RESET, miner->mining_pool_ip_address, miner->mining_pool_port);
        return EXIT_FAILURE;
    }
    else{
        printf(GRN "Mining Pool %s:%hu accepted solution!\n" RESET, miner->mining_pool_ip_address, miner->mining_pool_port);
        return EXIT_SUCCESS;
    }
}

int send_reward_transaction(miner_settings_t *miner, block_t *block){
    transaction_t *transaction;
    json_t *json_transaction;
    char *sender;
    int res;
    struct _u_request request;
    struct _u_response response;

    // prepare transaction
    transaction = (transaction_t*) malloc(sizeof(transaction_t));
    asprintf(&sender, "%0*x", HEX_LENGTH, 0);
    transaction->sender = sender;
    transaction->receiver = miner->rewarding_address;
    transaction->amount = block->coinbase;
    transaction->timestamp = (unsigned long int) time(NULL);

    // construct transaction
    json_transaction = json_construct_transaction(transaction);

    json_dump_file(json_transaction, "test_transaction.json", JSON_ENCODE_ANY);

    // send http request
    res = send_http_request(&request, &response, miner->mining_pool_ip_address, miner->mining_pool_port,
        ENDPOINT_MEMPOOL_ADD_HTTP_METHOD, ENDPOINT_MEMPOOL_ADD_URL_PATH, json_transaction, miner->request_timeout);

    if(res != U_OK){
        printf(RED "Mining Pool %s:%hu is down!\n" RESET, miner->mining_pool_ip_address, miner->mining_pool_port);
        return EXIT_FAILURE;
    }
    else{
        printf(GRN "Mining Pool %s:%hu accepted reward transaction!\n" RESET, miner->mining_pool_ip_address, miner->mining_pool_port);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

void print_stats(){
    time_t cur_time;
    int minutes;
    int hours;

    // time since miner started
    time(&cur_time);
    minutes = difftime(cur_time, start_time) / 60;
    hours = 0;
    while(minutes >= 60){
        hours++;
        minutes -= 60;
    }

    printf(CYN "Accepted Solutions: %d, Rejected Solutions: %d, Time: %02d:%02d\n\n" RESET, accepted, rejected, hours, minutes);
}
