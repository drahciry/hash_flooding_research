/**
 * @file hash_table_cw.h
 * @brief Hash Table implementation secured by Carter-Wegman Universal Hashing.
 * 
 * This module provides a hash table that leverages randomized hashing 
 * coefficients to mitigate Hash Flooding attacks, ensuring that a malicious 
 * actor cannot predictably force hash collisions to degrade performance.
 */

#ifndef HASH_TABLE_CW_H
#define HASH_TABLE_CW_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* 
 * If the CarterWegmanHasher is strictly managed inside hash_table_cw.c 
 * and is not passed as an argument by the user of this API, 
 * this include should ideally be moved to the .c file to decouple modules. 
 * Kept here assuming tight integration.
 */
#include "carter_wegman.h"

/**
 * @typedef HashTable
 * @brief Opaque pointer for the Universal Hash Table.
 * 
 * The internal arrays, capacity parameters, and the hashing context 
 * are entirely hidden to prevent unauthorized memory tampering and enforce 
 * strict encapsulation.
 */
typedef struct HashTable HashTable;

/**
 * @brief Allocates and initializes a new Hash Table secured with Universal Hashing.
 * 
 * @param[in] initial_capacity The starting number of buckets.
 * @return HashTable* Pointer to the newly created hash table, or NULL if 
 *         memory allocation fails.
 * 
 * @warning The caller assumes ownership of the returned pointer and must 
 *          safely deallocate it using ht_destroy() to prevent memory leaks.
 */
HashTable* ht_create(size_t initial_capacity);

/**
 * @brief Safely deallocates the entire hash table.
 * 
 * Frees all internal buckets, linked list nodes, and the embedded 
 * Carter-Wegman hashing context.
 * 
 * @param[in] hash_table Pointer to the hash table to be destroyed. If NULL, 
 *                       the function safely does nothing.
 */
void ht_destroy(HashTable* hash_table);

/**
 * @brief Inserts a new key-value pair into the hash table.
 * 
 * If the load factor exceeds the safe threshold, this function will 
 * automatically trigger an internal rehash to maintain O(1) performance.
 * 
 * @param[in,out] hash_table Pointer to the hash table instance.
 * @param[in]     key        The null-terminated string key to be hashed.
 * @param[in]     item       The 64-bit integer payload.
 * @return true if insertion was successful, false if a memory allocation 
 *         error occurred during internal operations.
 */
bool insertItem(HashTable* hash_table, const char* key, int64_t item);

/**
 * @brief Removes a key-value pair from the hash table.
 * 
 * Safely unlinks and frees the associated node memory, protecting the 
 * heap against Use-After-Free vulnerabilities.
 * 
 * @param[in,out] hash_table Pointer to the hash table instance.
 * @param[in]     key        The null-terminated string key to remove.
 * @return true if the item was found and deleted, false if the key does 
 *         not exist.
 */
bool deleteItem(HashTable* hash_table, const char* key);

/**
 * @brief Retrieves the value associated with a specific key.
 * 
 * @param[in]  hash_table Pointer to the hash table instance.
 * @param[in]  key        The null-terminated string key to search for.
 * @param[out] out_item   Pointer to a 64-bit integer where the retrieved 
 *                        item will be stored if found.
 * @return true if the key exists, false otherwise.
 */
bool getItem(HashTable* hash_table, const char* key, int64_t* out_item);

#endif /* HASH_TABLE_CW_H */