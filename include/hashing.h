#ifndef HASHING_H
#define HASHING_H

#define HASH_LENGTH 32
#define GROUP_LENGTH 2

// modulo functions
int modulo(long int value);
int modulo_add(long int valueX, long int valueY);

// conversion functions
int* hash_to_int_array(const char *hash_string);
char* int_array_to_hash(int *hash);

#endif // HASHING_H