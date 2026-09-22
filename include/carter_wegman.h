/**
 * @file carter_wegman.h
 * @brief Universal Hashing mitigation against Hash Flooding attacks.
 * 
 * This module provides a robust, randomized hashing context based on the 
 * Carter-Wegman algorithm. It ensures that hash functions are chosen 
 * randomly at runtime, preventing attackers from predicting bucket 
 * indices and forcing O(n) worst-case collision scenarios.
 */

#ifndef CARTER_WEGMAN_H
#define CARTER_WEGMAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @typedef CarterWegmanHasher
 * @brief Opaque pointer for the Carter-Wegman hashing context.
 * 
 * The internal structure is completely hidden from external modules to 
 * enforce strict memory encapsulation and protect cryptographic parameters 
 * from unauthorized access or modification.
 */
typedef struct CarterWegmanHasher CarterWegmanHasher;

/**
 * @brief Generates a cryptographically secure 64-bit random integer.
 * 
 * Reads from a secure entropy source (e.g., /dev/urandom) to generate 
 * values used as hashing coefficients.
 * 
 * @param[out] out_val Pointer to the memory address where the generated 
 *                     secure integer will be stored.
 * @return true if the random value was successfully generated, false if 
 *         the entropy source could not be read.
 */
bool generate_secure_uint64(uint64_t* out_val);

/**
 * @brief Generates an array of cryptographically secure 64-bit random integers.
 * 
 * Efficiently fills a pre-allocated array with secure random bytes in a 
 * single bulk operation, minimizing system call overhead.
 * 
 * @param[out] array Pointer to the pre-allocated array of 64-bit integers.
 * @param[in]  count The number of 64-bit integers to generate.
 * @return true if the bulk generation succeeded, false otherwise.
 */
bool generate_secure_bulk(uint64_t* array, size_t count);

/**
 * @brief Allocates and initializes a new Carter-Wegman hashing context.
 * 
 * @param[in] initial_capacity The starting capacity for the internal 
 *                             coefficient arrays.
 * @return A pointer to the newly allocated CarterWegmanHasher instance, 
 *         or NULL if the system is out of memory.
 * 
 * @warning The caller assumes ownership of the returned pointer and must 
 *          safely deallocate it using cw_destroy() to prevent memory leaks.
 */
CarterWegmanHasher* cw_create(size_t initial_capacity);

/**
 * @brief Safely deallocates the hashing context and its internal memory.
 * 
 * Frees all dynamic arrays associated with the hasher and the hasher 
 * structure itself.
 * 
 * @param[in] hasher Pointer to the CarterWegmanHasher instance to be destroyed. 
 *                   If hasher is NULL, the function does nothing.
 */
void cw_destroy(CarterWegmanHasher* hasher);

/**
 * @brief Computes the universal hash for a given data payload.
 * 
 * @param[in]  hasher   Pointer to the initialized Carter-Wegman context.
 * @param[in]  data     Pointer to the constant character array (payload) to hash.
 * @param[in]  len      The size of the data payload in bytes.
 * @param[out] raw_hash Pointer to the 32-bit integer where the resulting 
 *                      hash value will be written.
 * @return true if the hash was computed successfully, false if internal 
 *         capacity errors occurred.
 */
bool cw_hash(CarterWegmanHasher* hasher, const char* data, size_t len, uint32_t* raw_hash);

#endif /* CARTER_WEGMAN_H */