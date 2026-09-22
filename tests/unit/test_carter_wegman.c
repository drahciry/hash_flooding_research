#include "carter_wegman.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=================================================\n");
    printf("   CARTER-WEGMAN UNIVERSAL HASHING DEMONSTRATE   \n");
    printf("=================================================\n");

    CarterWegmanHasher* hasher1 = cw_create(16);
    CarterWegmanHasher* hasher2 = cw_create(16);

    if (!hasher1 || !hasher2) {
        printf("Error to initilize hashers.\n");
        return 1;
    }

    uint32_t raw_hash;
    const char* key1 = "Iniciacao_cientifica_2025";
    const char* key2 = "Iniciacao_cientifica_2026";

    printf("Key 1: '%s'\n", key1);
    printf("Key 2: '%s'\n\n", key2);

    printf("[Hasher 1 Instance] (Secret Key A)\n");
    if (cw_hash(hasher1, key1, strlen(key1), &raw_hash))
        printf("  -> Hash of '%s': %u\n", key1, raw_hash);
    if (cw_hash(hasher1, key2, strlen(key2), &raw_hash))
        printf("  -> Hash of '%s': %u\n\n", key2, raw_hash);

    printf("[Hasher 2 Instance] (Secret Key B - New Initialization)\n");
    if (cw_hash(hasher2, key1, strlen(key1), &raw_hash))
        printf("  -> Hash of '%s': %u\n", key1, raw_hash);
    if (cw_hash(hasher2, key2, strlen(key2), &raw_hash))
        printf("  -> Hash of '%s': %u\n\n", key2, raw_hash);

    cw_destroy(hasher1);
    cw_destroy(hasher2);

    return 0;
}