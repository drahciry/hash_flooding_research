#include "hash_table_dh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    /* 
     * Simulating the Hash Flooding vulnerability by loading 
     * a massive amount of deliberately colliding keys.
     */
    HashTable* hash_table = createHashTable(70000);
    if (!hash_table) {
        printf("Error allocating hash table.\n");
        return 1;
    }

    FILE* file = fopen("../results/collisions.txt", "r");
    if (file == NULL) { 
        printf("Error: Could not open collisions.txt. Please ensure the file exists.\n");
        deleteHashTable(hash_table);
        return 1;
    }

    printf("Starting bulk insertion of colliding keys...\n");

    char buffer[256];
    int count = 0;
    
    for (int i = 0; i < 65536; i++) {
        if (fgets(buffer, sizeof(buffer), file) == NULL) {
            break;
        }
        
        // Remove trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        
        insertItem(hash_table, buffer, i);
        count++;
    }

    printf("Total items inserted: %d\n", count);

    fclose(file);
    deleteHashTable(hash_table);

    return 0;
}