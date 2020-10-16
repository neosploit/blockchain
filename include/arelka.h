#ifndef ARELKA_H
#define ARELKA_H

#include <jansson.h>
#include "client.h"
#include "json.h"

void get_blockchain_sender_transactions(char *address);
void get_blockchain_receiver_transactions(char *address);
void get_block_id(int block_id);

#endif // ARELKA_H
