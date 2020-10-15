#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "../client/client.h"
#include "../json.h"

void get_transaction_info(client_settings_t *client, int *transaction_count, char **sender, char **receiver, char **amount, char **time, int *direction, int *status, int max_transaction_count);
bool balance_sufficient(client_settings_t *client, transaction_t* transaction);
int send_transaction(client_settings_t *client, transaction_t* transaction);

#endif // TRANSACTION_H