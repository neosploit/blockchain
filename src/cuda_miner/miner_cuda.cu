#include <jansson.h>
#include <time.h>
#include "miner_cuda.h"

extern "C"{
    #include "../../include/miner.h"
    #include "../../include/json.h"
    #include "../../include/hashing.h"
}

__global__ void calculate_block_hash_cuda(long int timestamp, int id, int *previous_hash, int *transaction_hashes, int transaction_count, unsigned int nonce_low, unsigned int nonce_high, int *hash_solution, unsigned int *nonce_solution){
    int i, j;
    unsigned int nonce;
    unsigned int temp_nonce;
    unsigned int nonce_splitted[NONCE_LENGTH / GROUP_LENGTH];
    int hash[HASH_LENGTH / GROUP_LENGTH];
    int zero_count;
    
    int thread_index = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int nonce_range = nonce_high - nonce_low;
    unsigned int low_index = nonce_low + thread_index * (nonce_range / (NUM_BLOCKS * NUM_THREADS));
    unsigned int high_index = nonce_low + (thread_index + 1) * (nonce_range / (NUM_BLOCKS * NUM_THREADS));
    
    // hashing algorithm
    for(nonce = low_index; nonce < high_index; nonce++){       
        // split nonce
        temp_nonce = nonce;
        for(i = 0; i < (NONCE_LENGTH / GROUP_LENGTH); i++){
            nonce_splitted[i] = temp_nonce % (1 << (4 * GROUP_LENGTH));
            temp_nonce /= (1 << (4 * GROUP_LENGTH));
        }
        
        // calculate block hash
        hash[0] = (long int)(timestamp + id) % (1 << (4 * GROUP_LENGTH));
        hash[0] = (hash[0] + (previous_hash[0] ^ nonce_splitted[0])) % (1 << (4 * GROUP_LENGTH));
        for(j = 0; j < transaction_count; j++){
        hash[0] = (hash[0] + (transaction_hashes[j*(HASH_LENGTH / GROUP_LENGTH)] ^ nonce_splitted[j % (NONCE_LENGTH / GROUP_LENGTH)])) % (1 << (4 * GROUP_LENGTH));
        }

        for(i = 1; i < HASH_LENGTH / GROUP_LENGTH; i++){
            hash[i] = (hash[i-1] + (previous_hash[i] ^ nonce_splitted[i % 4])) % (1 << (4 * GROUP_LENGTH));
            for(j = 0; j < transaction_count; j++){
                hash[i] = (hash[i] + (transaction_hashes[j*(HASH_LENGTH / GROUP_LENGTH) + i] ^ nonce_splitted[j % (NONCE_LENGTH / GROUP_LENGTH)])) % (1 << (4 * GROUP_LENGTH));
            }
        }

        // count zeros
        zero_count = 0;
        for(i = 0; i < HASH_LENGTH / GROUP_LENGTH; i++){
            if(hash[i] == 0){
                zero_count++;
            }
            else{
                break;
            }
        }

        // check if solution
        if(zero_count == DIFFICULTY / GROUP_LENGTH){
            for(i = 0; i < HASH_LENGTH / GROUP_LENGTH; i++){
                hash_solution[i] = hash[i];
            }
            *nonce_solution = nonce;
            break;
        }
    }
}

extern "C" int brute_force_solve_block(block_t *block){
    // cpu variables
    int *transaction_hashes;
    int *previous_hash;
    int *hash;
    unsigned int nonce;
    int i;
        
    // gpu variables
    int *transaction_hashes_gpu;
    int *previous_hash_gpu;
    int *hash_gpu;
    unsigned int *nonce_gpu;

    time_t end, start;
    double time_elapsed;
    int i_prev;

    time(&start);
    printf(BLU "\n%s" RESET, ctime(&start));
    printf(BLU "Starting Brute Force Mining of Block %d - Difficulty = %d\n" RESET, block->id, DIFFICULTY);

    // calculate transaction hashes
    transaction_hashes = calculate_transaction_hashes_1D(block->transactions, block->transaction_count);
    printf(CYN "\nCalculated Transaction Hashes\n" RESET);

    // previous hash
    previous_hash = hash_to_int_array(block->previous);

    // hash
    hash = (int*) calloc(HASH_LENGTH / GROUP_LENGTH, sizeof(int));

    // nonce
    nonce = 0;

    // allocate cuda memory
    cudaMalloc(&transaction_hashes_gpu, block->transaction_count * (HASH_LENGTH / GROUP_LENGTH) * sizeof(int*));
    cudaMalloc(&previous_hash_gpu, (HASH_LENGTH / GROUP_LENGTH) * sizeof(int));
    cudaMalloc(&hash_gpu, (HASH_LENGTH / GROUP_LENGTH) * sizeof(int));
    cudaMalloc(&nonce_gpu, sizeof(unsigned int));

    // copy from host to device
    cudaMemcpy(transaction_hashes_gpu, transaction_hashes, block->transaction_count * (HASH_LENGTH / GROUP_LENGTH) * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(previous_hash_gpu, previous_hash, HASH_LENGTH / GROUP_LENGTH * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(hash_gpu, hash, HASH_LENGTH / GROUP_LENGTH * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(nonce_gpu, &nonce, sizeof(unsigned int), cudaMemcpyHostToDevice);

    // solve block
    printf(BLU "Starting %d mining threads\n\n" RESET, NUM_BLOCKS * NUM_THREADS);

    i_prev = -1;

    for(i = 0; i < RANGE_PARTS; i++){
        // calculate hash for nonces range
        calculate_block_hash_cuda<<< NUM_BLOCKS, NUM_THREADS >>>(block->timestamp, block->id,
            previous_hash_gpu, transaction_hashes_gpu, block->transaction_count,
            i*(UINT_MAX/RANGE_PARTS), (i + 1)*(UINT_MAX/RANGE_PARTS), hash_gpu, nonce_gpu);
        
        // wait for gpu to finish
        cudaThreadSynchronize();

        // get solution
        cudaMemcpy(hash, hash_gpu, HASH_LENGTH / GROUP_LENGTH * sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemcpy(&nonce, nonce_gpu, sizeof(unsigned int), cudaMemcpyDeviceToHost);

        // hash string creation
        char *hash_string;
        hash_string = int_array_to_hash(hash);

        // print info
        time(&end);
        time_elapsed = difftime(end, start);

        if(time_elapsed >= MINER_CONSOLE_UPDATE_INTERVAL){
            printf(CYN "Hash Rate: %.2lf MH/s - Tried approx. %.2lf%% of all nonces\n" RESET,
                ((double)(i - i_prev) * (UINT_MAX/RANGE_PARTS)) / (1000000 * time_elapsed), (((double)(i + 1)/RANGE_PARTS) * 100));
            print_stats();

            time(&start);
        }
        
        // check if solution was found
        if(hash_ok(hash_string, DIFFICULTY)){
            printf(GRN "Found Nonce Solution: %u, Block Hash: %s!\n" RESET, nonce, hash_string);
            block->nonce = nonce;
            block->hash = hash_string;

            // cleanup
            cudaFree(transaction_hashes_gpu);
            cudaFree(previous_hash_gpu);
            cudaFree(hash_gpu);
            cudaFree(nonce_gpu);
            free(transaction_hashes);
            free(previous_hash);
            free(hash);

            return EXIT_SUCCESS;
        }        
    }          
    
    printf(RED "No Solution Found!\n" RESET);

    // cleanup
    cudaFree(transaction_hashes_gpu);
    cudaFree(previous_hash_gpu);
    cudaFree(hash_gpu);
    cudaFree(nonce_gpu);
    free(transaction_hashes);
    free(previous_hash);
    free(hash);

    return EXIT_FAILURE;
}