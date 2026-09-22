#include "hash_table_dh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Internal state macros for Open Addressing.
 * Kept private inside the .c file.
 */
#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

/*
 * Actual definition of the opaque pointer declared in hash_table_dh.h.
 */
struct HashTable {
    char** keys;
    int* items;
    char* status;
    unsigned long long R;
    unsigned long long size;
    unsigned long long inserteds;
};

/* ========================================================================= */
/* STATIC INTERNAL FUNCTIONS (Hidden from external modules)                  */
/* ========================================================================= */

static bool isPrime(unsigned long long num) {
    if (num <= 1) return false;
    for (unsigned long long divisor = 2; divisor * divisor <= num; divisor++) {
        if (num % divisor == 0) return false;
    }
    return true;
}

static unsigned long long primeBefore(unsigned long long size) {
    unsigned long long prime = size - 1;
    while (prime > 2) {
        if (isPrime(prime))
            return prime;
        prime--;
    }
    return 2;
}

static unsigned long long primeAfter(unsigned long long size) {
    unsigned long long prime = size + 1;
    while (true) {
        if (isPrime(prime))
            return prime;
        prime++;
    }
}

static unsigned long long djb2(const unsigned char* str) {
    unsigned long long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static unsigned long long hash1(HashTable* hash_table, const char* key) {
    unsigned long long k = djb2((const unsigned char*)key);
    return (k % hash_table->size);
}

static unsigned long long hash2(HashTable* hash_table, const char* key) {
    unsigned long long k = djb2((const unsigned char*)key);
    return (hash_table->R - (k % hash_table->R));
}

static unsigned long long doubleHash(HashTable* hash_table, const char* key, unsigned long long attempt) {
    return ((hash1(hash_table, key) + attempt * hash2(hash_table, key)) % hash_table->size);
}

/* Forward declaration for internal recursion during rehashing */
static void insertItemInternal(HashTable* hash_table, const char* key, int item, bool isRehashing);

static void rehash(HashTable* hash_table) {
    unsigned long long old_size = hash_table->size;
    unsigned long long new_size = primeAfter(old_size * 2);

    char** old_keys = hash_table->keys;
    int* old_items = hash_table->items;
    char* old_status = hash_table->status;

    hash_table->size = new_size;
    hash_table->R = primeBefore(hash_table->size);
    hash_table->inserteds = 0;

    hash_table->keys = (char**)calloc(hash_table->size, sizeof(char*));
    hash_table->items = (int*)malloc(hash_table->size * sizeof(int));
    hash_table->status = (char*)calloc(hash_table->size, sizeof(char));

    for (unsigned long long i = 0; i < old_size; i++) {
        if (old_status[i] == OCCUPIED) {
            insertItemInternal(hash_table, old_keys[i], old_items[i], true);
        } else if (old_status[i] == DELETED) {
            free(old_keys[i]);
        }
    }

    free(old_keys);
    free(old_items);
    free(old_status);
}

static void insertItemInternal(HashTable* hash_table, const char* key, int item, bool isRehashing) {
    double loadFactor = (double)hash_table->inserteds / hash_table->size;
    if (loadFactor >= 0.75) {
        rehash(hash_table);
    }

    unsigned long long hash = 0;
    unsigned long long deletedHash = 0;
    bool findDeletedHash = false;
    
    for (unsigned long long attempt = 0; attempt < hash_table->size; attempt++) {
        hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            break;
        } else if (hash_table->status[hash] == OCCUPIED) {
            if (strcmp(key, hash_table->keys[hash]) == 0) {
                hash_table->items[hash] = item;
                if (isRehashing) {
                    /* If rehashing, casting away const to free the old duplicated key if needed */
                    free((void*)key);
                }
                return;
            }
        } else if (hash_table->status[hash] == DELETED) {
            if (!findDeletedHash) {
                findDeletedHash = true;
                deletedHash = hash;
            }
        }
    }

    if (findDeletedHash) {
        hash = deletedHash;
        free(hash_table->keys[hash]);
    }

    if (isRehashing) {
        hash_table->keys[hash] = (char*)key; // Take ownership during rehash
    } else {
        hash_table->keys[hash] = strdup(key); // Duplicate the key for standard insertion
    }

    hash_table->inserteds++;
    hash_table->items[hash] = item;
    hash_table->status[hash] = OCCUPIED;
}

/* ========================================================================= */
/* PUBLIC API IMPLEMENTATIONS                                                */
/* ========================================================================= */

HashTable* createHashTable(unsigned long long size) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(struct HashTable));
    if (hash_table) {
        hash_table->inserteds = 0;
        hash_table->size = primeAfter(size);
        hash_table->R = primeBefore(hash_table->size);
        hash_table->keys = (char**)calloc(hash_table->size, sizeof(char*));
        hash_table->items = (int*)malloc(hash_table->size * sizeof(int));
        hash_table->status = (char*)calloc(hash_table->size, sizeof(char));
    }
    return hash_table;
}

void deleteHashTable(HashTable* hash_table) {
    if (!hash_table) return;

    for (unsigned long long i = 0; i < hash_table->size; i++) {
        if (hash_table->keys[i] != NULL) {
            free(hash_table->keys[i]);
        }
    }
    free(hash_table->keys);
    free(hash_table->items);
    free(hash_table->status);
    free(hash_table);
}

void insertItem(HashTable* hash_table, const char* key, int item) {
    insertItemInternal(hash_table, key, item, false);
}

bool getItem(HashTable* hash_table, const char* key, int* out_item) {
    for (unsigned long long attempt = 0; attempt < hash_table->size; attempt++) {
        unsigned long long hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            return false;
        } else if ((hash_table->status[hash] == OCCUPIED) && (strcmp(key, hash_table->keys[hash]) == 0)) {
            *out_item = hash_table->items[hash];
            return true;
        }
    }
    return false;
}

bool deleteItem(HashTable* hash_table, const char* key) {
    for (unsigned long long attempt = 0; attempt < hash_table->size; attempt++) {
        unsigned long long hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            return false;
        } else if ((hash_table->status[hash] == OCCUPIED) && (strcmp(key, hash_table->keys[hash]) == 0)) {
            /* BUG FIX: Changed '==' to '=' to correctly assign the DELETED status */
            hash_table->status[hash] = DELETED;
            hash_table->inserteds--;
            return true;
        }
    }
    return false;
}