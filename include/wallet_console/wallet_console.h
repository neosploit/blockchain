#ifndef WALLET_CONSOLE_H
#define WALLET_CONSOLE_H

#include "client.h"
#include "../json.h"

wallet_t* prepare_wallet();
wallet_t* recover_wallet();
int setup_wallet(client_settings_t client);
void add_wallet(client_settings_t client);
void print_wallet_info(client_settings_t client);
void print_wallet_balances(client_settings_t *client);

#endif // WALLET_CONSOLE_H