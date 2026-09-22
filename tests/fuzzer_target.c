/**
 * @file fuzzer_target.c
 * @brief Fuzzing harness for the Universal Hash Table.
 *
 * This file acts as the entry point for fuzzing campaigns (e.g., AFL++ 
 * or custom Python scripts). It reads payloads from standard input (stdin) 
 * and feeds them into the target API to monitor memory safety, execution 
 * time, and collision resistance against Hash Flooding.
 */

#include "hash_table_cw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * 1MB limit for fuzzer payloads. 
 * Prevents the harness itself from crashing due to heap exhaustion 
 * before the payload even reaches the target hash table.
 */
#define MAX_INPUT_BUFFER 1048576 

int main(int argc, char* argv[]) {
    /* 
     * Disable output buffering.
     * This is critical for fuzzing. If the program crashes (e.g., SegFault), 
     * the stdout buffer might not flush, and the Python orchestrator will lose 
     * the exact operation metrics that caused the crash.
     */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Instantiate the target via the public API */
    HashTable* table = ht_create(1024);
    if (!table) {
        fprintf(stderr, "FATAL: Failed to allocate target Hash Table.\n");
        return 1;
    }

    char* buffer = (char*)malloc(MAX_INPUT_BUFFER + 1);
    if (!buffer) {
        ht_destroy(table);
        fprintf(stderr, "FATAL: Failed to allocate fuzzer input buffer.\n");
        return 1;
    }

    uint64_t operation_count = 0;

    /* Fuzzing loop: Read payloads fed by the external fuzzer via stdin */
    while (fgets(buffer, MAX_INPUT_BUFFER + 1, stdin)) {
        /* Strip the newline character injected by line-based fuzzers */
        buffer[strcspn(buffer, "\n")] = 0;

        clock_t start_time = clock();

        /* Execute the vulnerable/mitigated operation we want to stress-test */
        bool success = insertItem(table, buffer, operation_count);

        clock_t end_time = clock();
        double elapsed_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

        /* Output metrics back to the Python orchestrator */
        printf("OP:%lu | SUCCESS:%d | TIME_MS:%f | SIZE:%zu\n",
               operation_count, success, elapsed_ms, table->size);

        operation_count++;
    }

    /* Clean up memory to ensure Valgrind doesn't report false positives */
    free(buffer);
    buffer = NULL;

    ht_destroy(table);
    table = NULL;

    return 0;
}