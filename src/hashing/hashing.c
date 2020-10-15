#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/hashing.h"

// modulo functions
int modulo(long int value){
    int hash;

    hash = value % (1 << (4 * GROUP_LENGTH));

    return hash;
}

int modulo_add(long int valueX, long int valueY){
    int hash;

    hash = (valueX + valueY) % (1 << (4 * GROUP_LENGTH));

    return hash;
}

// conversion functions
int* hash_to_int_array(const char *hash_string){
    int *hash;
    int i, j;
    char tmp[GROUP_LENGTH + 1];

    hash = (int*) calloc((HASH_LENGTH / GROUP_LENGTH), sizeof(int));

    for(i = 0; i < HASH_LENGTH / GROUP_LENGTH; i++){
        for(j = 0; j < GROUP_LENGTH; j++){
            tmp[j] = hash_string[2*i + j];
        }

        hash[i] = strtol(tmp, NULL, 16);
    }

    return hash;
}

char* int_array_to_hash(int *hash){
    char *hash_string;
    int i;
    char tmp[GROUP_LENGTH + 1];

    hash_string = (char*) malloc((HASH_LENGTH + 1) * sizeof(char));
    
    sprintf(hash_string, "%0*x", GROUP_LENGTH, hash[0]);
    for(i = 1; i < HASH_LENGTH/GROUP_LENGTH; i++){
        sprintf(tmp, "%0*x", GROUP_LENGTH, hash[i]);
        strcat(hash_string, tmp);
    }

    return hash_string;
}

