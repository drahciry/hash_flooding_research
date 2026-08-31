#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define PRIME 4294967291ULL

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    uint64_t* coefficients;
    size_t capacity;
} CarterWegmanHasher;

typedef struct Node {
    uint8_t* key;
    uint64_t item;
    Node* next;
} Node;

typedef struct {
    Node** buckets;
    uint8_t* status;
    size_t size;
    size_t capacity;
    CarterWegmanHasher* hasher;
} HashTable;

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
        uint64_t high = rand();
        uint64_t low = rand();
        uint64_t val = (high << 32) | low;
        hasher->coefficients[i] = val % PRIME;
    }

    return hasher;
}

HashTable* ht_create(size_t initial_capacity) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(HashTable));
    if (!hash_table) return NULL;

    hash_table->size = 0;
    hash_table->capacity = initial_capacity;

    hash_table->buckets = (Node**)malloc(hash_table->capacity * sizeof(Node*));
    hash_table->status = (uint8_t*)calloc(hash_table->capacity, sizeof(uint8_t));
    if (!hash_table->buckets || !hash_table->status) {
        if (hash_table->buckets) free(hash_table->buckets);
        if (hash_table->status) free(hash_table->status);
        free(hash_table);
        return NULL;
    }

    hash_table->hasher = cw_create(hash_table->capacity);
    if (!hash_table->hasher) {
        free(hash_table->buckets);
        free(hash_table->status);
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

uint32_t cw_hash(CarterWegmanHasher* hasher, const uint8_t* data, size_t len) {
    if (len == 0) return 0;

    if (!ensure_capacity(hasher, len)) {
        fprintf(stderr, "Allocation memory error to expand coefficients.\n");
        return 0;
    }

    uint64_t accum = 0;
    for (size_t i = 0; i < len; i++) {
        uint64_t term = (hasher->coefficients[i] * data[i]) % PRIME;
        accum = (accum + term) % PRIME;
    }

    return (uint32_t)accum;
}

void cw_destroy(CarterWegmanHasher* hasher) {
    if (hasher) {
        free(hasher->coefficients);
        free(hasher);
    }
}

void buckets_destroy(HashTable* hash_table) {
    for (size_t i = 0; i < hash_table->capacity; i++) {
        if (hash_table->status[i] == OCCUPIED) {
            Node* current = hash_table->buckets[i];
            while (current) {
                Node* next = current->next;
                free(current);
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
        free(hash_table->status);
        free(hash_table);
    }
}