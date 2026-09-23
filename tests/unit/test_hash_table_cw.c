#include "hash_table_cw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main() {
    /* 
     * Simulating the Hash Flooding vulnerability by loading 
     * a massive amount of deliberately colliding keys.
     */
    HashTable* hash_table = ht_create(70000);
    if (!hash_table) {
        printf("Error allocating hash table.\n");
        return 1;
    }

    /*
     * Reuse collisions used in test_hash_table_dh.c 
     */
    FILE* file = fopen("./results/collisions.txt", "r");
    if (file == NULL) { 
        printf("Error: Could not open collisions.txt. Please ensure the file exists.\n");
        ht_destroy(hash_table);
        return 1;
    }

    printf("Starting bulk insertion of keys...\n");

    char buffer[256];
    size_t count = 0;
    
    for (size_t i = 0; i < 65536; i++) {
        if (fgets(buffer, sizeof(buffer), file) == NULL) break;
        
        // Remove trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        
        insertItem(hash_table, buffer, i);
        count++;
    }

    #ifdef _WIN32
        printf("Total items inserted: %lld\n", count);
    #else
        printf("Total items inserted: %ld\n", count);
    #endif

    fclose(file);
    ht_destroy(hash_table);

    return 0;
}