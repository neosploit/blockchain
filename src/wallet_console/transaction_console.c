#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../wallet/wallet.h"
#include "../hashing.h"

transaction_t* prepare_transaction(client_settings_t client){
    json_t *wallets;
    json_t *j_wallet;
    size_t index;
    size_t choice;
    transaction_t *transaction;
    char buf[HEX_LENGTH + 1];
    double amount;
    char *sender;
    char *receiver;

    // choose sender address
    wallets = json_load_file(client.wallets_path, JSON_DECODE_ANY, NULL);
    
    printf("\nThere are %ld wallet addresses:\n", json_array_size(wallets));
    json_array_foreach(wallets, index, j_wallet){
        printf("%ld: %s\n", index, json_destruct_wallet(j_wallet).address);
    }

    do{
        printf("Choice: ");
        scanf("%ld", &choice);
    } while((choice < 0) || (choice >= json_array_size(wallets)));

    // allocate memory
    transaction = (transaction_t *) malloc(sizeof(transaction_t));
    sender = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));
    receiver = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));

    snprintf(sender, HEX_LENGTH + 1, "%s", json_destruct_wallet(json_array_get(wallets, choice)).address);

    transaction->sender = sender;

    // define receiver address
    do{
        printf("\nInsert Receiver: ");
        scanf("%s", buf);
    }while(!address_is_valid(buf));
    
    snprintf(receiver, HEX_LENGTH + 1, "%s", buf);
    transaction->receiver = receiver;

    // send amount
    printf("Amount: ");
    scanf("%lf", &amount);

    transaction->amount = amount;

    // timestamp from current time
    transaction->timestamp = (unsigned long int) time(NULL);

    return transaction;
}
