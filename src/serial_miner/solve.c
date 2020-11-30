#include <limits.h>
#include <string.h>
#include <time.h>
#include "../../include/miner.h"
#include "../../include/json.h"
#include "../../include/hashing.h"

int brute_force_solve_block(block_t *block){
    unsigned int nonce;
    char *hash;
    unsigned int **transaction_hashes;
    int i;    

    unsigned int nonce_solution;
    char *hash_solution;

    time_t end, start;
    double time_elapsed;
    unsigned int nonce_prev;

    time(&start);
    printf(BLU "\n%s" RESET, ctime(&start));
    printf(BLU "Starting Brute Force Mining of Block %d - Difficulty = %d\n" RESET, block->id, DIFFICULTY);

    // calculate transaction hashes
    transaction_hashes = calculate_transaction_hashes(block->transactions, block->transaction_count);
    printf(CYN "\nCalculated Transaction Hashes\n\n" RESET);

    // initialize
    nonce = 0;
    hash = NULL;
    nonce_solution = 0;
    hash_solution = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));
    nonce_prev = 0;

    // serial solve
    for(nonce = 0; nonce < UINT_MAX; nonce++){
        hash = calculate_block_hash(*block, transaction_hashes, nonce);
    
        if(hash_ok(hash, DIFFICULTY)){
            strncpy(hash_solution, hash, HEX_LENGTH + 1);
            nonce_solution = nonce;
            printf(GRN "Found Nonce Solution: %u, Block Hash: %s!\n" RESET, nonce_solution, hash_solution);
            break;            
        }
        else{
            free(hash);
            hash = NULL;
        }

        // print info
        time(&end);
        time_elapsed = difftime(end, start);

        if(time_elapsed >= MINER_CONSOLE_UPDATE_INTERVAL){
            printf(CYN "Hash Rate: %.2lf KH/s - Tried approx. %.2lf%% of all nonces\n" RESET,
                (double)(nonce - nonce_prev) / (1000 * time_elapsed), ((double)nonce / UINT_MAX) * 100);
            print_stats();
            time(&start);
            nonce_prev = nonce;
        }
    }

    // free memory
    for(i = 0; i < block->transaction_count; i++){
        free(transaction_hashes[i]);     
    }
    free(transaction_hashes);

    // no solution check
    if(!hash_ok(hash_solution, DIFFICULTY)){
        printf(RED "No Solution Found!\n" RESET);
        return EXIT_FAILURE;
    }

    block->nonce = nonce_solution;
    block->hash = hash_solution;
    return EXIT_SUCCESS;
}
