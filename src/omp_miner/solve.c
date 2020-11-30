#include <limits.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "../../include/miner.h"
#include "../../include/json.h"
#include "../../include/hashing.h"

#define THREAD_COUNT 8
#define RANGE_PARTS 256

void parallel_calculate_block_hash(block_t block, unsigned int **transaction_hashes, unsigned int nonce_low, unsigned int nonce_high, unsigned int *nonce_solution, char **hash_solution){
    unsigned int nonce;
    char *hash;

    // initialize
    nonce = 0;
    hash = NULL;

    // parallel calculatation of block hash
    #pragma omp parallel for private(nonce, hash), shared(block, transaction_hashes, nonce_solution, hash_solution) num_threads(THREAD_COUNT)
    for(nonce = nonce_low; nonce < nonce_high; nonce++){
        hash = calculate_block_hash(block, transaction_hashes, nonce);

        if(hash_ok(hash, DIFFICULTY)){
            #pragma omp critical
            {
                strncpy(hash_solution[0], hash, HEX_LENGTH + 1);
                *nonce_solution = nonce;
            }
        }
        else{
            free(hash);
            hash = NULL;
        }
    }
}

int brute_force_solve_block(block_t *block){
    unsigned int **transaction_hashes;
    int i;    
    
    unsigned int nonce_solution;
    char *hash_solution;

    time_t end, start;
    double time_elapsed;
    int i_prev;
    
    time(&start);
    printf(BLU "\n%s" RESET, ctime(&start));
    printf(BLU "Starting Brute Force Mining of Block %d - Difficulty = %d\n" RESET, block->id, DIFFICULTY);

    // calculate transaction hashes
    transaction_hashes = calculate_transaction_hashes(block->transactions, block->transaction_count);
    printf(CYN "\nCalculated Transaction Hashes\n" RESET);

    // initialize
    nonce_solution = 0;
    hash_solution = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));

    printf(BLU "Starting %d mining threads\n\n" RESET, THREAD_COUNT);

    i_prev = -1;

    // parallel solve
    for(i = 0; i < RANGE_PARTS; i++){
        // calculate hash for nonces range
        parallel_calculate_block_hash(*block, transaction_hashes, i*(UINT_MAX/RANGE_PARTS), (i+1)*(UINT_MAX/RANGE_PARTS),
            &nonce_solution, &hash_solution);
        
        // print info
        time(&end);
        time_elapsed = difftime(end, start);

        if(time_elapsed >= MINER_CONSOLE_UPDATE_INTERVAL){
            printf(CYN "Hash Rate: %.2lf KH/s - Tried approx. %.2lf%% of all nonces\n" RESET,
                ((double)(i - i_prev) * (UINT_MAX/RANGE_PARTS)) / (1000 * time_elapsed), (((double)(i + 1)/RANGE_PARTS) * 100));
            print_stats();
            time(&start);
            i_prev = i;
        } 

        // if found solution stop mining
        if(hash_ok(hash_solution, DIFFICULTY)){
            break;
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

    printf(GRN "Found Nonce Solution: %u, Block Hash: %s!\n" RESET, nonce_solution, hash_solution);
    block->nonce = nonce_solution;
    block->hash = hash_solution;
    return EXIT_SUCCESS;
}
