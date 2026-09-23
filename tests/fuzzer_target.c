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

int main() {
    /* 
     * Disable output buffering.
     * This is critical for fuzzing. If the program crashes (e.g., SegFault), 
     * the stdout buffer might not flush, and the Python orchestrator will lose 
     * the exact operation metrics that caused the crash.
     */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Lock the Carter-Wegman coefficients to a known state for reproducibility */
    cw_enable_deterministic(0xDEADBEEFCAFEBABE);

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
        #ifdef _WIN32
            printf("OP:%llu | SUCCESS:%d | TIME_MS:%f | SIZE:%zu\n",
                operation_count, success, elapsed_ms, ht_get_size(table));
        #else
            printf("OP:%lu | SUCCESS:%d | TIME_MS:%f | SIZE:%zu\n",
                operation_count, success, elapsed_ms, ht_get_size(table));
        #endif

        operation_count++;
    }

    free(buffer);
    buffer = NULL;

    ht_destroy(table);
    table = NULL;

    return 0;
}