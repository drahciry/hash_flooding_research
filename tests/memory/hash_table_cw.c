#include "hash_table_cw.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Memory Harness for Valgrind.
 * Purpose: Vigorously allocate and deallocate the hash table under heavy load 
 * to ensure all internal nodes, buckets, and cryptographic states are freed.
 */
int main() {
    HashTable* table = ht_create(1024);
    if (!table) return 1;

    // Simulate extreme insertion to force multiple internal reallocations
    for (int64_t i = 0; i < 50000; i++) {
        char key_buffer[32];
        snprintf(key_buffer, sizeof(key_buffer), "key_stress_%ld", i);
        insertItem(table, key_buffer, i);
    }

    // Force deletion of half the elements to test node_destroy limits
    for (int64_t i = 0; i < 25000; i++) {
        char key_buffer[32];
        snprintf(key_buffer, sizeof(key_buffer), "key_stress_%ld", i);
        deleteItem(table, key_buffer);
    }

    // This single call must recursively clean up the entire heap topology.
    // Valgrind will intercept this block to check for orphaned memory.
    ht_destroy(table);
    
    return 0;
}