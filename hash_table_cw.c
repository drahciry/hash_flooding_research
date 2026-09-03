/**
 * Important Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#else
    #include <sys/random.h>
#endif

/**
 * Macro definitions
 */

#define PRIME 4294967291ULL // Higher prime number before 2^32
#define DEFAULT_KEY_LENGTH 64 // default length to Carter Wegman Coefficients
#define MAX_KEY_LENGTH 1048576 // 1MB limit to prevent uint64_t overflow in cw_hash
#define LOAD_FACTOR_THRESHOLD 0.75 // load factor to measure if will be necessary expands

/**
 * Structs definitions:
 *     - Carter Wegman Hasher
 *     - Node to Linked List
 *     - Hash Table
 */

typedef struct {
    uint64_t* coefficients;
    size_t capacity;
} CarterWegmanHasher;

typedef struct Node {
    char* key;
    int64_t item;
    uint32_t hash;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    size_t size;
    size_t capacity;
    CarterWegmanHasher* hasher;
} HashTable;

/**
 * Auxiliary functions:
 *     - isPrime: returns if a number is prime
 *     - nextPrime: returns next prime based on entry
 *     - generate_secure_uint64: PRNG based on Operational System
 *     - node_destroy: free memory with Node struct
 *     - cw_destroy: free memory with Carter Wegman Hasher struct
 *     - buckest_destroy: free all memory with Node struct
 *     - ht_destroy: free memory with Hash Table struct
 *     - cw_create: allocate memory to Carter Wegman Hasher struct
 *     - ht_create: allocate memory to Hash Table struct
 *     - ensure_capacity: ensure that Carter Wegman Hasher capacity is sufficient
 */

bool isPrime(int64_t num) {
    if (num <= 1) return false;

    for (size_t i = 2; i * i <= num; i++)
        if (num % i == 0)
            return false;

    return true;
}

int64_t nextPrime(int64_t num) {
    if (num <= 0) num = 100;

    int64_t prime = num;

    while (!nextPrime(prime))
        prime++;

    return prime;
}

bool generate_secure_uint64(uint64_t* out_val) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(
        NULL,
        (PUCHAR)out_val,
        sizeof(uint64_t),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    return (status == 0);
#else
    ssize_t result = getrandom(out_val, sizeof(uint64_t), GRND_NONBLOCK);
    return (result == sizeof(uint64_t));
#endif
}

void node_destroy(Node* node) {
    free(node->key);
    free(node);
}

void cw_destroy(CarterWegmanHasher* hasher) {
    if (hasher) {
        free(hasher->coefficients);
        free(hasher);
    }
}

void buckets_destroy(HashTable* hash_table) {
    for (size_t i = 0; i < hash_table->capacity; i++) {
        if (hash_table->buckets[i]) {
            Node* current = hash_table->buckets[i];
            while (current) {
                Node* next = current->next;
                node_destroy(current);
                current = next;
            }
        }
    }
}

void ht_destroy(HashTable* hash_table) {
    if (hash_table) {
        cw_destroy(hash_table->hasher);
        buckets_destroy(hash_table);
        free(hash_table->buckets);
        free(hash_table);
    }
}

CarterWegmanHasher* cw_create(size_t initial_capacity) {
    CarterWegmanHasher* hasher = (CarterWegmanHasher*)malloc(sizeof(CarterWegmanHasher));
    if (!hasher) return NULL;
    
    hasher->coefficients = (uint64_t*)malloc(initial_capacity * sizeof(uint64_t));
    if (!hasher->coefficients) {
        free(hasher);
        return NULL;
    }
    hasher->capacity = initial_capacity;

    for (size_t i = 0; i < initial_capacity; i++) {
        uint64_t high; generate_secure_uint64(&high);
        uint64_t low; generate_secure_uint64(&low);
        uint64_t val = (high << 32) | low;
        hasher->coefficients[i] = val % PRIME;
    }

    return hasher;
}

HashTable* ht_create(size_t initial_capacity) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(HashTable));
    if (!hash_table) return NULL;

    initial_capacity = nextPrime(initial_capacity);

    hash_table->size = 0;
    hash_table->capacity = initial_capacity;

    hash_table->buckets = (Node**)calloc(hash_table->capacity, sizeof(Node*));
    hash_table->hasher = cw_create(DEFAULT_KEY_LENGTH);
    if (!hash_table->buckets || !hash_table->hasher) {
        if (hash_table->buckets) free(hash_table->buckets);
        if (hash_table->hasher) cw_destroy(hash_table->hasher);
        free(hash_table);
        return NULL;
    }

    return hash_table;
}

bool ensure_capacity(CarterWegmanHasher* hasher, uint64_t required_capacity) {
    if (required_capacity <= hasher->capacity) return true;

    uint64_t new_capacity = hasher->capacity << 1;
    if (new_capacity < required_capacity)
        new_capacity = required_capacity;

    uint64_t* new_coeffs = (uint64_t*)realloc(hasher->coefficients, new_capacity * sizeof(uint64_t));
    if (!new_coeffs) return false;

    hasher->coefficients = new_coeffs;

    for (size_t i = hasher->capacity; i < new_capacity; i++) {
        uint64_t high = rand();
        uint64_t low = rand();
        uint64_t val = (high << 32) | low;
        hasher->coefficients[i] = val % PRIME;
    }

    hasher->capacity = new_capacity;
    return true;
}

/**
 * Main functions:
 *     - cw_hash: generates a hash to string key with Carter Wegman Hasher
 *     - rehash: resizes the capacity to Hash Table struct
 *     - insertItem: inserts a item with key in Hash Table
 *     - deleteItem: deletes a item based on string key
 *     - getItem: returns item based on string key
 */

bool cw_hash(CarterWegmanHasher* hasher, const char* data, size_t len, uint32_t* raw_hash) {
    if (len == 0) return false;
    
    if (!ensure_capacity(hasher, len)) {
        fprintf(stderr, "Allocation memory error to expand coefficients.\n");
        return false;
    }

    uint64_t accum = 0;
    for (size_t i = 0; i < len; i++)
        accum += hasher->coefficients[i] * (uint8_t)data[i];
    
    *raw_hash = (uint32_t)(accum % PRIME);
    return true;
}

bool rehash(HashTable* hash_table) {
    uint64_t new_capacity = nextPrime(hash_table->capacity << 1);
    Node** new_buckets = (Node**)calloc(new_capacity, sizeof(Node*));
    if (!new_buckets) return false;

    for (size_t i = 0; i < hash_table->capacity; i++) {
        Node* current = hash_table->buckets[i];
        while (current) {
            Node* next = current->next;

            size_t index = current->hash % new_capacity;
            current->next = new_buckets[index];
            new_buckets[index] = current;

            current = next;
        }
    }

    free(hash_table->buckets);
    hash_table->buckets = new_buckets;
    hash_table->capacity = new_capacity;
    return true;
}

bool insertItem(HashTable* hash_table, const char* key, int64_t item) {
    double loadFactor = (double)hash_table->size / hash_table->capacity;
    if (loadFactor >= LOAD_FACTOR_THRESHOLD) rehash(hash_table);

    uint32_t raw_hash;
    size_t len = strlen(key);
    if (len > MAX_KEY_LENGTH) return false;

    if (!cw_hash(hash_table->hasher, key, strlen(key), &raw_hash))
        return false;
    size_t index = raw_hash % hash_table->capacity;

    Node** current = &(hash_table->buckets[index]);
    while (*current) {
        if ((*current)->hash == raw_hash && !strcmp((*current)->key, key)) {
            (*current)->item = item;
            return true;
        }
        current = &((*current)->next);
    }

    *current = (Node*)malloc(sizeof(Node));
    if (!(*current)) return false; 

    Node* newNode = *current;

    newNode->key = strdup(key);
    if (!newNode->key) {
        free(newNode);
        *current = NULL;
        return false;
    }

    newNode->item = item;
    newNode->hash = raw_hash;
    newNode->next = NULL;

    hash_table->size++;

    return true;
}

bool deleteItem(HashTable* hash_table, const char* key) {
    uint32_t raw_hash;
    if (!cw_hash(hash_table->hasher, key, strlen(key), &raw_hash))
        return false;
    size_t index = raw_hash % hash_table->capacity;

    if (hash_table->buckets[index] == NULL)
        return false;

    Node** current = &(hash_table->buckets[index]);
    while (*current) {
        if ((*current)->hash == raw_hash && !strcmp((*current)->key, key)) {
            Node* temp = *current;
            *current = temp->next;
            node_destroy(temp);
            hash_table->size--;
            return true;
        }
        current = &((*current)->next);
    }

    return false;
}

bool getItem(HashTable* hash_table, const char* key, int64_t* out_item) {
    uint32_t raw_hash;
    if (!cw_hash(hash_table->hasher, key, strlen(key), &raw_hash))
        return false;
    size_t index = raw_hash % hash_table->capacity;

    if (hash_table->buckets[index] == NULL)
        return false;

    Node** current = &(hash_table->buckets[index]);
    while (*current) {
        if ((*current)->hash == raw_hash && !strcmp((*current)->key, key)) {
            *out_item = (*current)->item;
            return true;
        }
        current = &((*current)->next);
    }

    return false;
}