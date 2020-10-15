#ifndef WALLET_H
#define WALLET_H

#include <stdbool.h>
#include <../client/client.h>
#include "../json.h"

#define WORD_COUNT 2048
#define WORD_LENGTH 20
#define SEED_PHRASE_LENGTH 16

// utility functions
int find_index_of_word(char *word);
int hash_of_word(char *word);
bool address_is_valid(char *address);

// key-address generation
char* generate_private_key(char **seed_phrase);
char* generate_public_key(const char *private_key);
char* generate_address(const char *public_key);

// wallet operations
int count_wallets(client_settings_t *client);
void get_wallet_info(client_settings_t *client, char **addresses, char **balances, int max_wallet_count);
double calculate_wallet_balance(client_settings_t *client, wallet_t wallet);
double calculate_confirmed_wallet_balance(client_settings_t *client, wallet_t wallet);

#endif // WALLET_H