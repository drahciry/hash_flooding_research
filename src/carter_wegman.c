#include "carter_wegman.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "bcrypt.lib")
    #endif
#else
    #include <sys/random.h>
#endif

/* 
 * Private macros. These are ONLY visible inside this .c file.
 */
#define PRIME 4294967291ULL // Higher prime number before 2^32
#define DEFAULT_KEY_LENGTH 64 // default length to Carter Wegman Coefficients

/*
 * Actual definition of the opaque pointer declared in the .h file.
 * Droping the 'typedef' here to avoid compiler redefinition errors.
 */
struct CarterWegmanHasher {
    uint64_t constant_b;
    uint64_t* coefficients;
    size_t capacity;
};

/* 
 * Internal state for deterministic fuzzing.
 */
static bool deterministic_mode = false;
static uint64_t prng_state = 1; // Xorshift state must never be 0

void cw_enable_deterministic(uint64_t seed) {
    deterministic_mode = true;
    prng_state = (seed == 0) ? 1 : seed;
}

/* 
 * Simple, fast Xorshift64 algorithm to generate reproducible pseudo-randomness.
 */
static uint64_t get_pseudo_random() {
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 7;
    prng_state ^= prng_state << 17;
    return prng_state;
}

/* 
 * Refactored Generation Functions to support the Seed 
 */
bool generate_secure_uint64(uint64_t* out_val) {
    if (deterministic_mode) {
        *out_val = get_pseudo_random();
        return true;
    }

#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(
        NULL,
        (PUCHAR)out_val,
        sizeof(uint64_t),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    return (status == 0);
#else
    ssize_t result = getrandom(out_val, sizeof(uint64_t), GRND_NONBLOCK);
    return (result == sizeof(uint64_t));
#endif
}

bool generate_secure_bulk(uint64_t* array, size_t count) {
    if (deterministic_mode) {
        for (size_t i = 0; i < count; i++) {
            array[i] = get_pseudo_random();
        }
        return true;
    }

    size_t total_bytes = count * sizeof(uint64_t);
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(
        NULL,
        (PUCHAR)array,
        total_bytes,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    return (status == 0);
#else
    ssize_t result = getrandom(array, total_bytes, GRND_NONBLOCK);
    return (result == (ssize_t)total_bytes);
#endif
}

void cw_destroy(CarterWegmanHasher* hasher) {
    if (hasher) {
        free(hasher->coefficients);
        free(hasher);
    }
}

CarterWegmanHasher* cw_create(size_t initial_capacity) {
    CarterWegmanHasher* hasher = (CarterWegmanHasher*)malloc(sizeof(struct CarterWegmanHasher));
    if (!hasher) return NULL;
    
    hasher->coefficients = (uint64_t*)malloc(initial_capacity * sizeof(uint64_t));
    if (!hasher->coefficients) {
        free(hasher);
        return NULL;
    }
    hasher->capacity = initial_capacity;

    if (!generate_secure_bulk(hasher->coefficients, hasher->capacity)) {
        cw_destroy(hasher);
        return NULL;
    }

    for (size_t i = 0; i < initial_capacity; i++)
        hasher->coefficients[i] %= PRIME;

    if (!generate_secure_uint64(&hasher->constant_b)) {
        cw_destroy(hasher);
        return NULL;
    };

    return hasher;
}

/* 
 * Declared as 'static' to enforce internal linkage. 
 * This function cannot be called from outside this file, protecting memory bounds.
 */
static bool ensure_capacity(CarterWegmanHasher* hasher, uint64_t required_capacity) {
    if (required_capacity <= hasher->capacity) return true;

    uint64_t new_capacity = hasher->capacity << 1;
    if (new_capacity < required_capacity)
        new_capacity = required_capacity;

    uint64_t* new_coeffs = (uint64_t*)realloc(hasher->coefficients, new_capacity * sizeof(uint64_t));
    if (!new_coeffs) return false;
    hasher->coefficients = new_coeffs;

    size_t new_elements = new_capacity - hasher->capacity;
    uint64_t* new_memory_start = &hasher->coefficients[hasher->capacity];

    if (!generate_secure_bulk(new_memory_start, new_elements)) {
        for (size_t i = 0; i < new_elements; i++)
            new_memory_start[i] = 1;
    }

    for (size_t i = hasher->capacity; i < new_capacity; i++)
        hasher->coefficients[i] %= PRIME;

    hasher->capacity = new_capacity;
    return true;
}

bool cw_hash(CarterWegmanHasher* hasher, const char* data, size_t len, uint32_t* raw_hash) {
    if (len == 0) return false;
    
    if (!ensure_capacity(hasher, len)) {
        fprintf(stderr, "Allocation memory error to expand coefficients.\n");
        return false;
    }

    uint64_t accum = hasher->constant_b;
    for (size_t i = 0; i < len; i++)
        accum += hasher->coefficients[i] * (uint8_t)data[i];
    
    *raw_hash = (uint32_t)(accum % PRIME);
    return true;
}