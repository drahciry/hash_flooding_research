/**
 * @file hash_table_dh.h
 * @brief Hash Table implementation using Double Hashing collision resolution.
 * 
 * This module provides a standard open-addressed hash table. It serves as 
 * the baseline for analyzing performance degradation during Hash Flooding 
 * attacks before randomized mitigations are applied.
 */

#ifndef HASH_TABLE_DH_H
#define HASH_TABLE_DH_H

#include <stdbool.h>

/**
 * @typedef HashTable
 * @brief Opaque pointer to hide the internal Double Hashing mechanics.
 * 
 * Prevents external modules from accessing internal arrays, probe sequence 
 * counters, or prime size configurations.
 */
typedef struct HashTable HashTable;

/**
 * @brief Allocates and initializes a new Hash Table using Double Hashing.
 * 
 * @param[in] size The initial requested size. The implementation will 
 *                 internally adjust this to the nearest prime number to 
 *                 ensure efficient probing sequences.
 * @return HashTable* Pointer to the newly created hash table, or NULL if OOM.
 * 
 * @warning The caller assumes ownership of the returned pointer and must 
 *          safely deallocate it using deleteHashTable().
 */
HashTable* createHashTable(unsigned long long size);

/**
 * @brief Safely deallocates the hash table and all its contents.
 * 
 * Returns the allocated heap memory to the operating system, preventing 
 * memory exhaustion during bulk testing.
 * 
 * @param[in] hash_table Pointer to the hash table to be destroyed.
 */
void deleteHashTable(HashTable* hash_table);

/**
 * @brief Inserts a new key-value pair into the hash table.
 * 
 * Handles collisions internally via Double Hashing sequences and triggers 
 * automatic rehashing if the table becomes too dense.
 * 
 * @param[in,out] hash_table Pointer to the hash table instance.
 * @param[in]     key        The constant string key to insert.
 * @param[in]     item       The integer value payload.
 */
void insertItem(HashTable* hash_table, const char* key, int item);

/**
 * @brief Retrieves the value associated with a specific key.
 * 
 * Traverses the Double Hashing probe sequence until the key is found or 
 * an empty bucket is encountered.
 * 
 * @param[in]  hash_table Pointer to the hash table instance.
 * @param[in]  key        The constant string key to search for.
 * @param[out] out_item   Pointer to store the retrieved item.
 * @return true if the key was found, false otherwise.
 */
bool getItem(HashTable* hash_table, const char* key, int* out_item);

/**
 * @brief Removes an item from the hash table.
 * 
 * Depending on the internal implementation, this may place a tombstone 
 * marker to preserve ongoing probe sequences.
 * 
 * @param[in,out] hash_table Pointer to the hash table instance.
 * @param[in]     key        The constant string key to delete.
 * @return true if the item was successfully deleted, false if not found.
 */
bool deleteItem(HashTable* hash_table, const char* key);

#endif /* HASH_TABLE_DH_H */