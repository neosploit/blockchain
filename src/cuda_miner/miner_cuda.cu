#include <jansson.h>
#include <time.h>
#include "miner_cuda.h"

extern "C"{
    #include "../../include/miner.h"
    #include "../../include/json.h"
    #include "../../include/hashing.h"
}

__device__ void hash_loop(unsigned int hash[], unsigned int words[]){
    int i;
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int m, k, temp;

    for (i = 0; i < (HEX_LENGTH / HEX_GROUP) * 8; i++){
        a = hash[0];
        b = hash[1];
        c = hash[2];
        d = hash[3];
        e = hash[4];
        f = hash[5];
        g = hash[6];
        h = hash[7];
        
        if (i < 16){
            m = (b & c) | ((~ b) & d) | (b & (~e)) | ((~b) & (~f)) | (b & (~g)) | ((~b) & h);
            k = 1518500249; // 0x5A827999
        }
        else if (i < 32){
            m = b ^ c ^ d ^ e ^ f ^ g ^ h;
            k = 1859775393; // 0x6ED9EBA1
        }
        else if (i < 48){
            m = (b & c) | (b & d) | (b & e) | (b & f) | (b & g) | (b & h);
            k = 2400959708; // 0x8F1BBCDC
        }
        else{
            m = b ^ c ^ d ^ e ^ f ^ g ^ h;
            k = 3395469782; // 0xCA62C1D6
        }

        temp = ((a << 5) | (a >> 27)) ^ (m & k) ^ words[i];

        h = g;
        g = (f << 29) | (f >> 3);
        f = e;
        e = d;
        d = c;
        c = (b << 13) | (b >> 19);
        b = a;
        a = temp;

        hash[0] ^= a;
        hash[1] ^= b;
        hash[2] ^= c;
        hash[3] ^= d;
        hash[4] ^= e;
        hash[5] ^= f;
        hash[6] ^= g;
        hash[7] ^= h;
    }
}

__global__ void calculate_block_hash_cuda(unsigned long int timestamp, unsigned int id, unsigned int *previous_hash, unsigned int *transaction_hashes, int transaction_count, unsigned int nonce_low, unsigned int nonce_high, unsigned int *hash_solution, unsigned int *nonce_solution){
    int i, j;
    unsigned int nonce;
    unsigned int hash[HEX_LENGTH / HEX_GROUP];
    unsigned int words[(HEX_LENGTH / HEX_GROUP) * 8];
    
    unsigned int thread_index = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int nonce_range = nonce_high - nonce_low;
    unsigned int low_index = nonce_low + thread_index * (nonce_range / (NUM_BLOCKS * NUM_THREADS));
    unsigned int high_index = nonce_low + (thread_index + 1) * (nonce_range / (NUM_BLOCKS * NUM_THREADS));
    
    // hashing algorithm
    for(nonce = low_index; nonce < high_index; nonce++){  

        /* Step 1 - Init Hash with Previous Block's Hash */
        for (i = 0; i < (int)(HEX_LENGTH / HEX_GROUP); i++){
            hash[i] = previous_hash[i];
        }

        /* Step 2 - Add Block Info */
        words[0] = nonce;
        words[1] = (timestamp >> 32) % UINT_MAX;
        words[2] = timestamp % UINT_MAX;
        words[3] = id;
        
        for (i = 4; i < (HEX_LENGTH / HEX_GROUP) * 8; i++){
            words[i] = ((words[i - 2] ^ words[i - 4]) << 3) | ((words[i - 2] ^ words[i - 4]) >> 29);
        }

        hash_loop(hash, words);

        /* Step 3 - Add Transaction Hashes */
        for (i = 0; i < transaction_count; i++){
            for (j = 0; j < (HEX_LENGTH / HEX_GROUP); j++){
                words[j] = transaction_hashes[i * (HEX_LENGTH / HEX_GROUP) + j];
            }
            
            for (j = (HEX_LENGTH / HEX_GROUP); j < (HEX_LENGTH / HEX_GROUP) * 8; j++){
                words[j] = ((words[j - 2] ^ words[j - 4]) << 3) | ((words[j - 2] ^ words[j - 4]) >> 29);
            }

            hash_loop(hash, words);
        }
        
        // check if solution
        if(DIFFICULTY == 8){
            if(hash[0] == 0){
                for(i = 0; i < HEX_LENGTH / HEX_GROUP; i++){
                    hash_solution[i] = hash[i];
                }
                *nonce_solution = nonce;
                break;
            }
        }
        else if(hash[0] <= (1 << (32 - DIFFICULTY*4) - 1)){
            for(i = 0; i < HEX_LENGTH / HEX_GROUP; i++){
                hash_solution[i] = hash[i];
            }
            *nonce_solution = nonce;
            break;
        }
    }
}

extern "C" int brute_force_solve_block(block_t *block){
    // cpu variables
    unsigned int *transaction_hashes;
    unsigned int *previous_hash;
    unsigned int *hash;
    unsigned int nonce;
    long int i;
        
    // gpu variables
    unsigned int *transaction_hashes_gpu;
    unsigned int *previous_hash_gpu;
    unsigned int *hash_gpu;
    unsigned int *nonce_gpu;

    time_t end, start;
    double time_elapsed;
    long int i_prev;

    time(&start);
    printf(BLU "\n%s" RESET, ctime(&start));
    printf(BLU "Starting Brute Force Mining of Block %d - Difficulty = %d\n" RESET, block->id, DIFFICULTY);

    // calculate transaction hashes
    transaction_hashes = calculate_transaction_hashes_1D(block->transactions, block->transaction_count);
    printf(CYN "\nCalculated Transaction Hashes\n" RESET);

    // previous hash
    previous_hash = hash_to_int_array(block->previous);

    // hash
    hash = (unsigned int*) calloc(HEX_LENGTH / HEX_GROUP, sizeof(unsigned int));

    // nonce
    nonce = 0;

    // allocate cuda memory
    cudaMalloc(&transaction_hashes_gpu, block->transaction_count * (HEX_LENGTH / HEX_GROUP) * sizeof(unsigned int));
    cudaMalloc(&previous_hash_gpu, (HEX_LENGTH / HEX_GROUP) * sizeof(unsigned int));
    cudaMalloc(&hash_gpu, (HEX_LENGTH / HEX_GROUP) * sizeof(unsigned int));
    cudaMalloc(&nonce_gpu, sizeof(unsigned int));

    // copy from host to device
    cudaMemcpy(transaction_hashes_gpu, transaction_hashes, block->transaction_count * (HEX_LENGTH / HEX_GROUP) * sizeof(unsigned int), cudaMemcpyHostToDevice);
    cudaMemcpy(previous_hash_gpu, previous_hash, HEX_LENGTH / HEX_GROUP * sizeof(unsigned int), cudaMemcpyHostToDevice);
    cudaMemcpy(hash_gpu, hash, HEX_LENGTH / HEX_GROUP * sizeof(unsigned int), cudaMemcpyHostToDevice);
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
        cudaDeviceSynchronize();

        // get solution
        cudaMemcpy(hash, hash_gpu, HEX_LENGTH / HEX_GROUP * sizeof(unsigned int), cudaMemcpyDeviceToHost);
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
            i_prev = i;

            time(&start);
        }
        
        // check if solution was found
        if(hash_ok(hash_string, DIFFICULTY)){
            printf(CYN "Hash Rate: %.2lf MH/s - Tried approx. %.2lf%% of all nonces\n" RESET,
                ((double)(i - i_prev) * (UINT_MAX/RANGE_PARTS)) / (1000000 * time_elapsed), (((double)(i + 1)/RANGE_PARTS) * 100));
            print_stats();
            time(&start);
            i_prev = i;

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
