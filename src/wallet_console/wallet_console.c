#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <jansson.h>
#include "../wallet_console/client.h"
#include "../wallet/wallet.h"
#include "../auxilary.h"
#include "../json.h"
#include "../hashing.h"

extern const char bib0039[WORD_COUNT][WORD_LENGTH];

wallet_t* prepare_wallet(){
    char **seed_phrase;
    int index;
    int i;
    char buf[80];
    char *name;

    wallet_t* wallet;

    wallet = (wallet_t *) malloc(1 * sizeof(wallet_t));
    
    srand(time(NULL));

    // wallet name
    printf("\nEnter wallet name: ");
    scanf("%s", buf);

    name = (char*) malloc((strlen(buf) + 1) * sizeof(char));
    strncpy(name, buf, strlen(buf) + 1);
    wallet->name = name;
    
    // seed phrase generation
    printf("\nGenerating seed phrase...\n");

    seed_phrase = (char**) malloc(SEED_PHRASE_LENGTH * sizeof(char*));

    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        index =  rand() % WORD_COUNT;

        seed_phrase[i] = (char*) malloc((strlen(bib0039[index]) + 1) * sizeof(char));

        sprintf(seed_phrase[i], "%s", bib0039[index]);
    }

    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        printf("%d : %s\n", i, seed_phrase[i]);
    }
    printf("\n");
    
    // generate keys
    wallet->private_key = generate_private_key(seed_phrase);
    printf("Private Key: %s\n", wallet->private_key);

    wallet->public_key = generate_public_key(wallet->private_key);
    printf("Public Key: %s\n", wallet->public_key);

    wallet->address = generate_address(wallet->public_key);
    printf("Wallet Address: %s\n", wallet->address);

    // free memory
    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        free(seed_phrase[i]);
    }
    free(seed_phrase);

    return wallet;
}

wallet_t* recover_wallet(){
    char **seed_phrase;
    int i;

    char buf[80];
    char *name;

    wallet_t* wallet;

    wallet = (wallet_t *) malloc(1 * sizeof(wallet_t));

    // wallet name
    printf("\nEnter wallet name: ");
    scanf("%80s", buf);

    name = (char*) malloc((strlen(buf) + 1) * sizeof(char));
    strncpy(name, buf, strlen(buf) + 1);
    wallet->name = name;

    // seed phrase input
    seed_phrase = (char**) malloc(SEED_PHRASE_LENGTH * sizeof(char*));

    printf("Enter seed phrase...\n");

    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        printf("%d: ", i);
        scanf("%20s", buf);

        seed_phrase[i] = (char*) malloc((strlen(buf) + 1) * sizeof(char));

        sprintf(seed_phrase[i], "%s", buf);
    }

    // check if seed word false
    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        if(find_index_of_word(seed_phrase[i]) == -1){
            return NULL;
        }
    }

    // generate keys
    wallet->private_key = generate_private_key(seed_phrase);
    printf("Private Key: %s\n", wallet->private_key);

    wallet->public_key = generate_public_key(wallet->private_key);
    printf("Public Key: %s\n", wallet->public_key);

    wallet->address = generate_address(wallet->public_key);
    printf("Wallet Address: %s\n", wallet->address);

    // free memory
    for(i = 0; i < SEED_PHRASE_LENGTH; i++){
        free(seed_phrase[i]);
    }
    free(seed_phrase);

    return wallet;
}

int setup_wallet(client_settings_t client){
    json_t *wallets;
    wallet_t *wallet;
    char input;

    wallets = json_load_file(client.wallets_path, JSON_DECODE_ANY, NULL);

    if (json_array_size(wallets) == 0){

    	do{
	    	printf("Create(c/C) or Recover(r/R) Wallet?\n");
            while(getchar() != '\n');
            scanf("%c", &input);
	    	
	    	switch(input){
	    		case 'c':
	    		case 'C':
	    			wallet = prepare_wallet();
	    			break;
	    		case 'r':
	    		case 'R':
	    			wallet = recover_wallet();
	    			break;
	    		default:
	    			printf("Input not c/C or r/R!\n");
			}
			
		} while(input != 'c' && input != 'C' && input != 'r' && input != 'R');
    	
	    json_array_append_new(wallets, json_construct_wallet(wallet));
	    
		json_dump_file(wallets, client.wallets_path, JSON_ENCODE_ANY);
	}

    return EXIT_SUCCESS;
}

void add_wallet(client_settings_t client){
    json_t *wallets;
    wallet_t *wallet;
    char input;
                
    wallets = json_load_file(client.wallets_path, JSON_DECODE_ANY, NULL);
    
    do{
        printf("Create(c/C) or Recover(r/R) Wallet?\n");
        while(getchar() != '\n');
        scanf("%c", &input);
        
        switch(input){
            case 'c':
            case 'C':
                wallet = prepare_wallet();
                break;
            case 'r':
            case 'R':
                wallet = recover_wallet();
                break;
            default:
                printf("Input not c/C or r/R!\n");
        }
    } while(input != 'c' && input != 'C' && input != 'r' && input != 'R');
    
    json_array_append_new(wallets, json_construct_wallet(wallet));
    
    json_dump_file(wallets, client.wallets_path, JSON_ENCODE_ANY);
}

void print_wallet_info(client_settings_t client){
    json_t *wallets;
    json_t *wallet_json;
    wallet_t wallet;
    size_t index;

    wallets = json_load_file(client.wallets_path, JSON_DECODE_ANY, NULL);

    printf("\n%ld wallets have been created.\n\n", json_array_size(wallets));
    json_array_foreach(wallets, index, wallet_json){
        wallet = json_destruct_wallet(wallet_json);

        printf("Wallet %ld:\n", index);
        printf("Name: %s\n", wallet.name);
        printf("Public Key: %s\n", wallet.public_key);
        printf("Address: %s\n\n", wallet.address);
    }
}

void print_wallet_balances(client_settings_t *client){
    json_t *blockchain;
    json_t *block_json;
    block_t block;
    size_t block_index;

    json_t *mempool;
    json_t *transaction_json;
    transaction_t transaction;
    size_t mempool_index;

    json_t *wallets;
    json_t *wallet_json;
    wallet_t wallet;
    size_t wallet_index;

    double *wallet_balances;
    double *unconfirmed_balances;
    size_t wallet_count;

    size_t i;

    // read json files
    wallets = json_load_file(client->wallets_path, JSON_DECODE_ANY, NULL);
    blockchain = json_load_file(client->blockchain_path, JSON_DECODE_ANY, NULL);
    mempool = json_load_file(client->mempool_path, JSON_DECODE_ANY, NULL);

    // wallets counts
    wallet_count = json_array_size(wallets);

    // allocate memory for balances
    wallet_balances = (double*) calloc(wallet_count, sizeof(double));
    unconfirmed_balances = (double*) calloc(wallet_count, sizeof(double));

    // for each block
    json_array_foreach(blockchain, block_index, block_json){
        block = json_destruct_block(block_json);

        // for each transaction
        for(i = 0; i < block.transaction_count; i++){

            // for each wallet address
            json_array_foreach(wallets, wallet_index, wallet_json){
                wallet = json_destruct_wallet(wallet_json);

                // check if address is sender
                if(!strcmp(block.transactions[i].sender, wallet.address)){
                    wallet_balances[wallet_index] -= block.transactions[i].amount;
                }

                // check if address is receiver
                if(!strcmp(block.transactions[i].receiver, wallet.address)){
                    wallet_balances[wallet_index] += block.transactions[i].amount;
                }
            }            
        }
    }
    
    // for each unconfirmed transaction
    json_array_foreach(mempool, mempool_index, transaction_json){
        transaction = json_destruct_transaction(transaction_json);

        // for each wallet address
        json_array_foreach(wallets, wallet_index, wallet_json){
            wallet = json_destruct_wallet(wallet_json);

            // check if address is sender
            if(!strcmp(transaction.sender, wallet.address)){
                unconfirmed_balances[wallet_index] -= transaction.amount;
            }

            // check if address is receiver
            if(!strcmp(transaction.receiver, wallet.address)){
                unconfirmed_balances[wallet_index] += transaction.amount;
             }
        }
    }

    printf("\nWallet Balances are :\n");
    for(i = 0; i < wallet_count; i++){
        printf("%ld: %.2lf (+ %.2lf unconfirmed)\n", i, wallet_balances[i], unconfirmed_balances[i]);
    }
}