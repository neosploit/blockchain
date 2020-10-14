#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include "sha256/mark2/sha256.h"
#include "client.h"
#include "auxilary.h"
#include "objects.h"

int prepare_wallet() {
    wallet_t wallet;
    char wallet_name[33];
    char wallet_password[33], wallet_private_key[65];
    uint8_t hash[SHA256_BYTES];
    sha256_context ctx;

    // Ask for wallet name
    fprintf(stdout, "\nWallet name: ");
    if (!fgets(wallet_name, sizeof(wallet_name), stdin)) return -1;

    // Ask for wallet password
    fprintf(stdout, "\nWallet password: ");
    if (!fgets(wallet_password, sizeof(wallet_password), stdin)) return -1;

    // Generate private key
    sha256(wallet_password, strlen(wallet_password), hash);
    for (int i = 0; i < SHA256_BYTES; ++i) *wallet_private_key += sprintf(wallet_private_key, "%02x", hash[i]);

    // Save wallet info
    asprintf(wallet.name, wallet_name);
    asprintf(wallet.private_key, wallet_private_key);

    // Finally, create wallet
    // if (create_wallet(&wallet) == -1) {

    // }

    return 0;
}

int create_wallet(const client_settings_t *client, const wallet_t *wallet) {
    const json_t *json_wallet = json_construct_wallet(wallet);
    json_t *json_loaded_wallets;

    if (!json_loaded_wallets = json_load_file(client->))


}