/**
 * @file hash_table.c
 * @brief Memory harness for Valgrind targeting the Double Hashing implementation.
 */

#include "hash_table_dh.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    /* Create a deliberately small table to force early rehashing */
    HashTable* hash_table = createHashTable(10);
    if (!hash_table) {
        fprintf(stderr, "Failed to allocate hash table.\n");
        return 1;
    }

    /* Original user test cases */
    insertItem(hash_table, "user", 1413914);
    insertItem(hash_table, "secure_password", 12345);
    insertItem(hash_table, "born_date", 20050104);

    int item;
    if (getItem(hash_table, "secure_password", &item)) {
        printf("Secure password: %d\n", item);
    }

    deleteItem(hash_table, "secure_password");
    insertItem(hash_table, "secure_password", 12345);

    /* 
     * STRESS TEST FOR VALGRIND
     * We insert 5,000 items to force the internal arrays to realloc multiple times.
     * This ensures Valgrind checks if the old arrays are properly freed.
     */
    char buffer[32];
    for (int i = 0; i < 5000; i++) {
        snprintf(buffer, sizeof(buffer), "stress_key_%d", i);
        insertItem(hash_table, buffer, i);
    }

    /* 
     * We delete half of them to test the DELETED status logic and ensure 
     * that strdup'd keys are properly freed without double-free errors.
     */
    for (int i = 0; i < 2500; i++) {
        snprintf(buffer, sizeof(buffer), "stress_key_%d", i);
        deleteItem(hash_table, buffer);
    }

    /* Final cleanup. Valgrind will report any orphaned memory here. */
    deleteHashTable(hash_table);

    return 0;
}