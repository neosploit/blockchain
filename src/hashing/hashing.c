#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/hashing.h"

// conversion functions
unsigned int* hash_to_int_array(const char *hash_string){
    unsigned int *hash;
    int i, j;
    char tmp[HEX_GROUP + 1];

    hash = (unsigned int*) calloc((int)(HEX_LENGTH / HEX_GROUP), sizeof(unsigned int));

    for(i = 0; i < (int)(HEX_LENGTH / HEX_GROUP); i++){
        for(j = 0; j < HEX_GROUP; j++){
            tmp[j] = hash_string[2*i + j];
        }
        
        hash[i] = strtol(tmp, NULL, 16);
    }

    return hash;
}

char* int_array_to_hash(unsigned int *hash){
    char *hash_string;
    int i;
    char tmp[HEX_GROUP + 1];

    hash_string = (char*) malloc((HEX_LENGTH + 1) * sizeof(char));
    
    sprintf(hash_string, "%0*x", HEX_GROUP, hash[0]);
    for(i = 1; i < HEX_LENGTH/HEX_GROUP; i++){
        sprintf(tmp, "%0*x", HEX_GROUP, hash[i]);
        strcat(hash_string, tmp);
    }

    return hash_string;
}

