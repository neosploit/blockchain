#ifndef HASHING_H
#define HASHING_H

#define BIT_LENGTH 256
#define HEX_LENGTH ((int) (BIT_LENGTH / 4))
#define HEX_GROUP  ((int) sizeof(unsigned int) * 2)

// conversion functions
unsigned int* hash_to_int_array(const char *hash_string);
char* int_array_to_hash(unsigned int *hash);

#endif // HASHING_H
