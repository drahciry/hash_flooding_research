#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    char** keys;   
    int* items;    
    char* status;  
    int R;         
    int size;      
    int inserteds; 
} HashTable;

bool isPrime(int num) {
    if (num == 1) return false;
    for (int divisor = 2; divisor * divisor <= num; divisor++)
        if (num % divisor == 0) return false;
    return true;
}

int primeBefore(int size) {
    int prime = size - 1;
    while (prime > 2) {
        if (isPrime(prime))
            return prime;
        prime--;
    }
    return 2;
}

int primeAfter(int size) {
    int prime = size + 1;
    while (true) {
        if (isPrime(prime))
            return prime;
        prime++;
    }
}

unsigned long djb2Hash(unsigned char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

int hash1(HashTable* hash_table, char* key) {
    unsigned long k = djb2Hash((unsigned char*)key);
    return (k % hash_table->size);
}

int hash2(HashTable* hash_table, char* key) {
    unsigned long k = djb2Hash((unsigned char*)key);
    return (hash_table->R - (k % hash_table->R));
}

int doubleHash(HashTable* hash_table, char* key, int attempt) {
    return ((hash1(hash_table, key) + attempt * hash2(hash_table, key)) % hash_table->size);
}

void insertItemInternal(HashTable* hash_table, char* key, int item, bool isRehashing);

void rehash(HashTable* hash_table) {
    int old_size = hash_table->size;
    int new_size = primeAfter(old_size * 2);

    char** old_keys = hash_table->keys;
    int* old_items = hash_table->items;
    char* old_status = hash_table->status;

    hash_table->size = new_size;
    hash_table->R = primeBefore(new_size);
    hash_table->inserteds = 0;

    hash_table->keys = (char**)calloc(hash_table->size, sizeof(char*));
    hash_table->items = (int*)malloc(hash_table->size * sizeof(int));
    hash_table->status = (char*)calloc(hash_table->size, sizeof(char));

    for (int i = 0; i < old_size; i++) {
        if (old_status[i] == OCCUPIED)
            insertItemInternal(hash_table, old_keys[i], old_items[i], true);
        else if (old_status[i] == DELETED)
            free(old_keys[i]);
    }

    free(old_keys);
    free(old_items);
    free(old_status);
}

void insertItemInternal(HashTable* hash_table, char* key, int item, bool isRehashing) {
    float loadFactor = (float)hash_table->inserteds / hash_table->size;
    if (loadFactor >= 0.75)
        rehash(hash_table);

    int hashDeleted = -1;
    int hash;

    for (int attempt = 0; attempt < hash_table->size; attempt++) {
        hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            break;
        } else if (hash_table->status[hash] == OCCUPIED) {
            if (strcmp(key, hash_table->keys[hash]) == 0) {
                hash_table->items[hash] = item;
                if (isRehashing) free(key);
                return;
            }
        } else {
            if (hashDeleted == -1) hashDeleted = hash;
        }
    }

    if (hashDeleted != -1) {
        hash = hashDeleted;
        free(hash_table->keys[hash]);
        hash_table->keys[hash] = NULL;
    }

    if (isRehashing)
        hash_table->keys[hash] = key;
    else
        hash_table->keys[hash] = strdup(key);

    hash_table->items[hash] = item;
    hash_table->status[hash] = OCCUPIED;
    hash_table->inserteds++;
}

void insertItem(HashTable* hash_table, char* key, int item) {
    insertItemInternal(hash_table, key, item, false);
}

bool getItem(HashTable* hash_table, char* key, int* out_item) {
    for (int attempt = 0; attempt < hash_table->size; attempt++) {
        int hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            return false;
        } else if ((hash_table->status[hash] == OCCUPIED) && (strcmp(key, hash_table->keys[hash]) == 0)) {
            *out_item = hash_table->items[hash];
            return true;
        }
    }
    return false;
}

bool deleteItem(HashTable* hash_table, char* key) {
    for (int attempt = 0; attempt < hash_table->size; attempt++) {
        int hash = doubleHash(hash_table, key, attempt);
        if (hash_table->status[hash] == EMPTY) {
            return false;
        } else if ((hash_table->status[hash] == OCCUPIED) && (strcmp(key, hash_table->keys[hash]) == 0)) {
            hash_table->status[hash] = DELETED;
            hash_table->inserteds--;
            return true;
        }
    }
    return false;
}

HashTable* createHashTable(int size) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(HashTable));
    if (hash_table) {
        hash_table->inserteds = 0;
        hash_table->size = primeAfter(size);
        hash_table->R = primeBefore(hash_table->size);
        hash_table->keys = (char**)calloc(hash_table->size, sizeof(char*));
        hash_table->items = (int*)malloc(hash_table->size * sizeof(int));
        hash_table->status = (char*)calloc(hash_table->size, sizeof(char));
    }
    return hash_table;
}

void deleteHashTable(HashTable* hash_table) {
    for (int i = 0; i < hash_table->size; i++)
        if (hash_table->keys[i] != NULL)
            free(hash_table->keys[i]);
    free(hash_table->keys);
    free(hash_table->items);
    free(hash_table->status);
    free(hash_table);
}

int main() {
    HashTable* hash_table = createHashTable(70000);

    FILE* file = fopen("collisions.txt", "r");
    if (file == NULL) { 
        printf("Error to open file.\n");
        return 1;
    }

    printf("Beginning of inserts.\n");

    char buffer[50];
    for (int i = 0; i < 65536; i++) {
        if (fgets(buffer, sizeof(buffer), file) == NULL) break;
        buffer[strcspn(buffer, "\n")] = '\0';
        insertItem(hash_table, buffer, i);
    }

    printf("Total inserts: %d\n", hash_table->inserteds);

    fclose(file);
    deleteHashTable(hash_table);
    hash_table = NULL;

    return 0;
}