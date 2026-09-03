#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#else
    #include <sys/random.h>
#endif

#define PRIME 4294967291ULL // Higher prime number before 2^32
#define DEFAULT_KEY_LENGTH 64 // default length to Carter Wegman Coefficients

typedef struct {
    uint64_t constant_b;
    uint64_t* coefficients;
    size_t capacity;
} CarterWegmanHasher;

bool generate_secure_uint64(uint64_t* out_val) {
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

void cw_destroy(CarterWegmanHasher* hasher) {
    if (hasher) {
        free(hasher->coefficients);
        free(hasher);
    }
}

CarterWegmanHasher* cw_create(size_t initial_capacity) {
    CarterWegmanHasher* hasher = (CarterWegmanHasher*)malloc(sizeof(CarterWegmanHasher));
    if (!hasher) return NULL;
    
    hasher->coefficients = (uint64_t*)malloc(initial_capacity * sizeof(uint64_t));
    if (!hasher->coefficients) {
        free(hasher);
        return NULL;
    }
    hasher->capacity = initial_capacity;

    for (size_t i = 0; i < initial_capacity; i++) {
        uint64_t val = 0;
        if (!generate_secure_uint64(&val)) {
            cw_destroy(hasher);
            return NULL;
        }
        hasher->coefficients[i] = val % PRIME;
    }

    if (!generate_secure_uint64(&hasher->constant_b)) {
        cw_destroy(hasher);
        return NULL;
    };

    return hasher;
}

bool ensure_capacity(CarterWegmanHasher* hasher, uint64_t required_capacity) {
    if (required_capacity <= hasher->capacity) return true;

    uint64_t new_capacity = hasher->capacity << 1;
    if (new_capacity < required_capacity)
        new_capacity = required_capacity;

    uint64_t* new_coeffs = (uint64_t*)realloc(hasher->coefficients, new_capacity * sizeof(uint64_t));
    if (!new_coeffs) return false;

    hasher->coefficients = new_coeffs;

    for (size_t i = hasher->capacity; i < new_capacity; i++) {
        uint64_t val = 0;
        if (!generate_secure_uint64(&val)) val = 1;
        hasher->coefficients[i] = val % PRIME;
    }

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
