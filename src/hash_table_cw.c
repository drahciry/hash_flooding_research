#include "hash_table_cw.h"
#include "carter_wegman.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define DEFAULT_KEY_LENGTH 64      // Default initial length for Carter-Wegman coefficients
#define MAX_KEY_LENGTH 1048576     // 1MB limit to prevent uint64_t overflow in cw_hash
#define LOAD_FACTOR_THRESHOLD 0.75 // Load factor threshold to trigger automatic rehashing

/*
 * Internal Node structure for handling collisions via Chaining.
 * Completely hidden from the public API.
 */
typedef struct Node {
    char* key;
    int64_t item;
    uint32_t hash;
    struct Node* next;
} Node;

/*
 * Actual definition of the opaque pointer declared in hash_table_cw.h.
 */
struct HashTable {
    Node** buckets;
    size_t size;
    size_t capacity;
    CarterWegmanHasher* hasher; // Pointer to external cryptographic module
};

/* ========================================================================= */
/* STATIC INTERNAL FUNCTIONS (Hidden from external linkage)                  */
/* ========================================================================= */

static bool isPrime(int64_t num) {
    if (num <= 1) return false;
    for (size_t i = 2; i * i <= (size_t)num; i++) {
        if (num % i == 0)
            return false;
    }
    return true;
}

static int64_t nextPrime(int64_t num) {
    if (num <= 0) num = 100;
    int64_t prime = num;
    while (!isPrime(prime)) {
        prime++;
    }
    return prime;
}

static void node_destroy(Node* node) {
    if (node) {
        free(node->key);
        free(node);
    }
}

static void buckets_destroy(HashTable* hash_table) {
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

static bool rehash(HashTable* hash_table) {
    uint64_t new_capacity = nextPrime(hash_table->capacity << 1);
    Node** new_buckets = (Node**)calloc(new_capacity, sizeof(Node*));
    if (!new_buckets) return false;

    for (size_t i = 0; i < hash_table->capacity; i++) {
        Node* current = hash_table->buckets[i];
        while (current) {
            Node* next = current->next;

            size_t index = current->hash % new_capacity;
            
            // Re-link the node into the new bucket array
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

/* ========================================================================= */
/* PUBLIC API IMPLEMENTATIONS                                                */
/* ========================================================================= */

HashTable* ht_create(size_t initial_capacity) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(struct HashTable));
    if (!hash_table) return NULL;

    initial_capacity = nextPrime(initial_capacity);

    hash_table->size = 0;
    hash_table->capacity = initial_capacity;

    hash_table->buckets = (Node**)calloc(hash_table->capacity, sizeof(Node*));
    
    // Instantiate the external Universal Hasher securely
    hash_table->hasher = cw_create(DEFAULT_KEY_LENGTH);
    
    if (!hash_table->buckets || !hash_table->hasher) {
        if (hash_table->buckets) free(hash_table->buckets);
        if (hash_table->hasher) cw_destroy(hash_table->hasher);
        free(hash_table);
        return NULL;
    }

    return hash_table;
}

void ht_destroy(HashTable* hash_table) {
    if (hash_table) {
        // Delegate hashing context destruction to its own API
        cw_destroy(hash_table->hasher);
        
        // Destroy internal nodes
        buckets_destroy(hash_table);
        
        // Destroy the bucket array and the table itself
        free(hash_table->buckets);
        free(hash_table);
    }
}

bool insertItem(HashTable* hash_table, const char* key, int64_t item) {
    double loadFactor = (double)hash_table->size / hash_table->capacity;
    if (loadFactor >= LOAD_FACTOR_THRESHOLD) {
        rehash(hash_table);
    }

    size_t len = strlen(key);
    if (len == 0 || len > MAX_KEY_LENGTH) return false;
    
    uint32_t raw_hash;
    
    // Call the external API to generate the universal hash
    if (!cw_hash(hash_table->hasher, key, len, &raw_hash)) {
        return false;
    }
        
    size_t index = raw_hash % hash_table->capacity;

    // Traverse the linked list looking for an existing key to update
    Node** current = &(hash_table->buckets[index]);
    while (*current) {
        if ((*current)->hash == raw_hash && !strcmp((*current)->key, key)) {
            (*current)->item = item;
            return true;
        }
        current = &((*current)->next);
    }

    // Key not found, allocate a new node
    *current = (Node*)malloc(sizeof(Node));
    if (!(*current)) return false; 

    Node* newNode = *current;
    
    // duplicate the string to take ownership of the memory inside the table
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
    size_t len = strlen(key);
    if (len == 0 || len > MAX_KEY_LENGTH) return false;

    uint32_t raw_hash;
    if (!cw_hash(hash_table->hasher, key, len, &raw_hash)) {
        return false;
    }
        
    size_t index = raw_hash % hash_table->capacity;

    if (hash_table->buckets[index] == NULL) {
        return false;
    }

    Node** current = &(hash_table->buckets[index]);
    while (*current) {
        if ((*current)->hash == raw_hash && !strcmp((*current)->key, key)) {
            Node* temp = *current;
            *current = temp->next; // Bypass the node
            node_destroy(temp);    // Safely free the memory
            hash_table->size--;
            return true;
        }
        current = &((*current)->next);
    }

    return false;
}

bool getItem(HashTable* hash_table, const char* key, int64_t* out_item) {
    size_t len = strlen(key);
    if (len == 0 || len > MAX_KEY_LENGTH) return false;

    uint32_t raw_hash;
    if (!cw_hash(hash_table->hasher, key, len, &raw_hash)) {
        return false;
    }
        
    size_t index = raw_hash % hash_table->capacity;

    if (hash_table->buckets[index] == NULL) {
        return false;
    }

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