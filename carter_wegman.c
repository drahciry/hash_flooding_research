#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define PRIME 4294967291ULL

typedef struct {
    uint64_t *coefficients;
    size_t capacity;
} CarterWegmanHasher;

CarterWegmanHasher* cw_create(size_t initial_capacity) {
    CarterWegmanHasher* hasher = (CarterWegmanHasher*)malloc(sizeof(CarterWegmanHasher));
    if (!hasher) return NULL;

    hasher->coefficients = (uint64_t*)malloc(initial_capacity * sizeof(uint64_t));
    if (!hasher->coefficients) {
        free(hasher);
        return NULL;
    }
    hasher->capacity  = initial_capacity;

    for (size_t i = 0; i < initial_capacity; i++) {
        uint64_t high = rand();
        uint64_t low = rand();
        uint64_t val = (high << 32) | low;
        hasher->coefficients[i] = val % PRIME;
    }

    return hasher;
}

bool cw_ensure_capacity(CarterWegmanHasher* hasher, size_t required_capacity) {
    if (required_capacity <= hasher->capacity) return true;

    size_t new_capacity = hasher->capacity << 1;
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }

    uint64_t* new_coeffs = realloc(hasher->coefficients, new_capacity * sizeof(uint64_t));
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
    if  (len == 0) return 0;

    if (!cw_ensure_capacity(hasher, len)) {
        fprintf(stderr, "Allocation memory error to expands coefficients.\n");
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

int main() {
    srand(time(NULL));

    printf("=================================================\n");
    printf("   CARTER-WEGMAN UNIVERSAL HASHING DEMONSTRATE   \n");
    printf("=================================================\n");

    CarterWegmanHasher* hasher1 = cw_create(16);

    srand(time(NULL) ^ 0xdeadbeef);
    CarterWegmanHasher* hasher2 = cw_create(16);

    if (!hasher1 || !hasher2) {
        printf("Error to initilize hashers.\n");
        return 1;
    }

    const char* key1 = "Iniciacao_cientifica_2025";
    const char* key2 = "Iniciacao_cientifica_2026";

    printf("Key 1: '%s'\n", key1);
    printf("Key 2: '%s'\n\n", key2);

    printf("[Hasher 1 Instance] (Secret Key A)\n");
    printf("  -> Hash of '%s': %u\n", key1, cw_hash(hasher1, (const uint8_t*)key1, strlen(key1)));
    printf("  -> Hash of '%s': %u\n\n", key2, cw_hash(hasher1, (const uint8_t*)key2, strlen(key2)));

    printf("[Hasher 2 Instance] (Secret Key B - New Initialization)\n");
    printf("  -> Hash of '%s': %u\n", key1, cw_hash(hasher2, (const uint8_t*)key1, strlen(key1)));
    printf("  -> Hash of '%s': %u\n\n", key2, cw_hash(hasher2, (const uint8_t*)key2, strlen(key2)));

    cw_destroy(hasher1);
    cw_destroy(hasher2);

    return 0;
}
