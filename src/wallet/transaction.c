#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/wallet/transaction.h"
#include "../../include/wallet/wallet.h"
#include "../../include/client/endpoints.h"
#include "../../include/client/client.h"
#include "../../include/json.h"
#include "../../include/auxilary.h"
#include "../../include/hashing.h"

void get_transaction_info(client_settings_t *client, int *transaction_count, char **sender, char **receiver, char **amount, char **time, int *direction, int *status, int max_transaction_count){
    json_t *blockchain;
    json_t *block_json;
    block_t block;

    json_t *mempool;
    json_t *transaction_json;
    transaction_t transaction;

    json_t *wallets;
    json_t *wallet_json;
    wallet_t wallet;
    size_t wallet_index;
    
    time_t timestamp;
    struct tm *timestruct;

    // init
    blockchain = json_null();
    mempool = json_null();
    wallets = json_null();
    *transaction_count = 0;
    
    // read json files
    wallets = json_load_file(client->wallets_path, JSON_DECODE_ANY, NULL);
    blockchain = json_load_file(client->blockchain_path, JSON_DECODE_ANY, NULL);
    mempool = json_load_file(client->mempool_path, JSON_DECODE_ANY, NULL);
    
    // for each unconfirmed transaction
    for(int mempool_index = json_array_size(mempool) - 1; mempool_index >= 0; mempool_index--){
        transaction_json = json_array_get(mempool, mempool_index);
        transaction = json_destruct_transaction(transaction_json);

        // for each wallet address
        json_array_foreach(wallets, wallet_index, wallet_json){
            wallet = json_destruct_wallet(wallet_json);

            // check if address is sender
            if(!strcmp(transaction.sender, wallet.address)){
                if(*transaction_count == 0){
                    sprintf(sender[*transaction_count], "%s", transaction.sender);
                    sprintf(receiver[*transaction_count], "%s", transaction.receiver);
                    sprintf(amount[*transaction_count], "%.2lf", transaction.amount);
                    timestamp = transaction.timestamp;
                    timestruct = localtime(&timestamp);
                    sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                    direction[*transaction_count] = 0;
                    status[*transaction_count] = 0;
                }
                else{
                    sprintf(sender[*transaction_count], "%s", transaction.sender);
                    sprintf(receiver[*transaction_count], "%s", transaction.receiver);
                    sprintf(amount[*transaction_count], "%.2lf", transaction.amount);
                    timestamp = transaction.timestamp;
                    timestruct = localtime(&timestamp);
                    sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                    direction[*transaction_count] = 0;
                    status[*transaction_count] = 0;
                }
                *transaction_count += 1;
                if(*transaction_count >= max_transaction_count) goto CLEAN;
            }

            // check if address is receiver
            if(!strcmp(transaction.receiver, wallet.address)){
                if(*transaction_count == 0){
                    sprintf(sender[*transaction_count], "%s", transaction.sender);
                    sprintf(receiver[*transaction_count], "%s", transaction.receiver);
                    sprintf(amount[*transaction_count], "%.2lf", transaction.amount);
                    timestamp = transaction.timestamp;
                    timestruct = localtime(&timestamp);
                    sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                    direction[*transaction_count] = 1;
                    status[*transaction_count] = 0;
                }
                else{
                    sprintf(sender[*transaction_count], "%s", transaction.sender);
                    sprintf(receiver[*transaction_count], "%s", transaction.receiver);
                    sprintf(amount[*transaction_count], "%.2lf", transaction.amount);
                    timestamp = transaction.timestamp;
                    timestruct = localtime(&timestamp);
                    sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                    direction[*transaction_count] = 1;
                    status[*transaction_count] = 0;
                }
                *transaction_count += 1;
                if(*transaction_count >= max_transaction_count) goto CLEAN;
            }
        }
    }
    
    // for each block
    for(int block_index = json_array_size(blockchain) - 1; block_index >= 0; block_index--){
        block_json = json_array_get(blockchain, block_index);
        block = json_destruct_block(block_json);

        // for each transaction
        for(int i = block.transaction_count - 1; i >= 0; i--){
            // for each wallet address
            json_array_foreach(wallets, wallet_index, wallet_json){
                wallet = json_destruct_wallet(wallet_json);

                // check if address is sender
                if(!strcmp(block.transactions[i].sender, wallet.address)){
                    if(*transaction_count == 0){
                        sprintf(sender[*transaction_count], "%s", block.transactions[i].sender);
                        sprintf(receiver[*transaction_count], "%s", block.transactions[i].receiver);
                        sprintf(amount[*transaction_count], "%.2lf", block.transactions[i].amount);
                        timestamp = block.transactions[i].timestamp;
                        timestruct = localtime(&timestamp);
                        sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                        direction[*transaction_count] = 0;
                        status[*transaction_count] = 1;
                    }
                    else{                        
                        sprintf(sender[*transaction_count], "%s", block.transactions[i].sender);
                        sprintf(receiver[*transaction_count], "%s", block.transactions[i].receiver);
                        sprintf(amount[*transaction_count], "%.2lf", block.transactions[i].amount);
                        timestamp = block.transactions[i].timestamp;
                        timestruct = localtime(&timestamp);
                        sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                        direction[*transaction_count] = 0;
                        status[*transaction_count] = 1;
                    }
                    *transaction_count += 1;
                    if(*transaction_count >= max_transaction_count) goto CLEAN;
                }
                
                // check if address is receiver
                if(!strcmp(block.transactions[i].receiver, wallet.address)){
                    if(*transaction_count == 0){
                        sprintf(sender[*transaction_count], "%s", block.transactions[i].sender);
                        sprintf(receiver[*transaction_count], "%s", block.transactions[i].receiver);
                        sprintf(amount[*transaction_count], "%.2lf", block.transactions[i].amount);
                        timestamp = block.transactions[i].timestamp;
                        timestruct = localtime(&timestamp);
                        sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                        direction[*transaction_count] = 1;
                        status[*transaction_count] = 1;
                    }
                    else{
                        sprintf(sender[*transaction_count], "%s", block.transactions[i].sender);
                        sprintf(receiver[*transaction_count], "%s", block.transactions[i].receiver);
                        sprintf(amount[*transaction_count], "%.2lf", block.transactions[i].amount);
                        timestamp = block.transactions[i].timestamp;
                        timestruct = localtime(&timestamp);
                        sprintf(time[*transaction_count], "%d-%02d-%02d %02d:%02d:%02d",
                            timestruct->tm_year + 1900, timestruct->tm_mon, timestruct->tm_mday,
                            timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                        direction[*transaction_count] = 1;
                        status[*transaction_count] = 1;
                    }
                    *transaction_count += 1;
                    if(*transaction_count >= max_transaction_count) goto CLEAN;
                }
            }            
        }
    }

CLEAN:
    /* Cleanup */
    if(!json_is_null(wallets)){
        json_array_clear(wallets);
        json_decref(wallets);
    }
    if(!json_is_null(blockchain)){
        json_array_clear(blockchain);
        json_decref(blockchain);
    }
    if(!json_is_null(mempool)){
        json_array_clear(mempool);
        json_decref(mempool);
    }
}

bool balance_sufficient(client_settings_t *client, transaction_t* transaction){
    json_t *wallets;
    json_t *wallet_json;
    wallet_t wallet;
    size_t wallet_index;

    // init
    wallets = NULL;

    // read json file
    wallets = json_load_file(client->wallets_path, JSON_DECODE_ANY, NULL);

    // find wallet
    json_array_foreach(wallets, wallet_index, wallet_json){
        wallet = json_destruct_wallet(wallet_json);
        if(!strcmp(wallet.address, transaction->sender)){
            break;
        }
    }

    /* cleanup */
    if(wallets != NULL)
        free(wallets);

    // check if balance is sufficient
    if(calculate_wallet_balance(client, wallet) >= transaction->amount){
        return true;
    }
    
    return false;
}

int send_transaction(client_settings_t *client, transaction_t* transaction){
    json_t *json_transaction;
    struct _u_request request;
    struct _u_response response;
    json_t *nodes;
    json_t *node_json;
    node_t node;
    size_t node_index;

    if(!balance_sufficient(client, transaction)){
        return EXIT_FAILURE;
    }
    else if(transaction->amount <= 0){
        return EXIT_FAILURE;
    }

    // init
    nodes = NULL;

    // construct transaction
    json_transaction = json_construct_transaction(transaction);

    json_dump_file(json_transaction, "test_transaction.json", JSON_ENCODE_ANY);

    // send http request to local endpoint
    send_http_request(&request, &response, CLIENT_DEFAULT_IP_ADDRESS, client->port,
        ENDPOINT_MEMPOOL_ADD_HTTP_METHOD, ENDPOINT_MEMPOOL_ADD_URL_PATH, json_transaction, client->request_timeout);

    // read nodes file
    nodes = json_load_file(client->nodes_path, JSON_DECODE_ANY, NULL);

    // send http request to all known nodes
    json_array_foreach(nodes, node_index, node_json){
        node = json_destruct_node(node_json);

        send_http_request(&request, &response, node.ip_address, node.port,
            ENDPOINT_MEMPOOL_ADD_HTTP_METHOD, ENDPOINT_MEMPOOL_ADD_URL_PATH, json_transaction, client->request_timeout);
    }

    /* Cleanup */
    if(nodes != NULL)
        free(nodes);

    return EXIT_SUCCESS;
}