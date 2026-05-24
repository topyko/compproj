#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tables.h"

#define INITIAL_CAPACITY 16    // Initial number of slots in the symbol table.
#define LOAD_FACTOR 0.75       // Maximum load factor before resizing (rehashing).

/**
 * Calculates the hash value for a given string.
 * Uses the djb2 hash algorithm.
 * 
 * @param str The input string to hash.
 * @param capacity The current capacity of the table (used for modulo operation).
 * @return The hash index within the table's capacity.
 */
unsigned int hash(const char* str, int capacity) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % capacity;
}

/**
 * Creates a new symbol table with initial capacity.
 * 
 * @return Pointer to the newly created SymbolTable.
 */
SymbolTable* create_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->entries = calloc(INITIAL_CAPACITY, sizeof(Var));  // Allocate entries array.
    table->used = calloc(INITIAL_CAPACITY, sizeof(int));     // Allocate usage tracking array.
    table->size = 0;                                         // Initially, the table is empty.
    table->capacity = INITIAL_CAPACITY;
    return table;
}

/**
 * Frees all memory associated with the given symbol table.
 * 
 * @param table Pointer to the symbol table to free.
 */
void free_table(SymbolTable* table) {
    free(table->entries);
    free(table->used);
    free(table);
}

/**
 * Resizes the symbol table when it exceeds the allowed load factor.
 * Rehashes all existing entries into the new table.
 * 
 * @param table Pointer to the symbol table to rehash.
 */
void rehash(SymbolTable* table) {
    int old_capacity = table->capacity;
    Var* old_entries = table->entries;
    int* old_used = table->used;

    table->capacity *= 2;  // Double the capacity.
    table->entries = calloc(table->capacity, sizeof(Var));  // Allocate new entries array.
    table->used = calloc(table->capacity, sizeof(int));     // Allocate new usage array.
    table->size = 0;  // Reset size (will be updated as we re-add entries).

    // Reinsert each old entry into the new table.
    for (int i = 0; i < old_capacity; i++) {
        if (old_used[i]) {
            table_add(table, old_entries[i].name, old_entries[i].type);
        }
    }

    free(old_entries);
    free(old_used);
}

/**
 * Adds a new variable to the symbol table.
 * If the variable already exists, an error is thrown.
 * If the load factor is exceeded, the table is rehashed.
 * 
 * @param table Pointer to the symbol table.
 * @param name The name of the variable to add.
 * @param type The type of the variable.
 */
void table_add(SymbolTable* table, Identifier name, TypeEnum type) {
    // Check if rehashing is needed.
    if ((float)table->size / table->capacity > LOAD_FACTOR) {
        rehash(table);
    }

    unsigned int index = hash(name, table->capacity);

    // Handle collisions using linear probing.
    while (table->used[index]) {
        if (strcmp(table->entries[index].name, name) == 0) {
            fprintf(stderr, "ERROR: redeclaration of variable '%s'\n", name);
            exit(1);  // Prevent redeclaration.
        }
        index = (index + 1) % table->capacity;  // Move to the next slot.
    }

    // Insert the new variable.
    table->entries[index].name = name;
    table->entries[index].type = type;
    table->used[index] = 1;  // Mark slot as used.
    table->size++;
}

/**
 * Searches for a variable in the symbol table.
 * 
 * @param table Pointer to the symbol table.
 * @param name The name of the variable to search for.
 * @param found Output parameter: set to 1 if the variable is found, or 0 if not found.
 * @return The type of the variable if found; returns -1 if not found.
 */
TypeEnum table_lookup(SymbolTable* table, Identifier name, int* found) {
    unsigned int index = hash(name, table->capacity);
    int start_index = index;

    // Search using linear probing.
    do {
        if (table->used[index] && strcmp(table->entries[index].name, name) == 0) {
            *found = 1;  // Variable found.
            return table->entries[index].type;
        }
        index = (index + 1) % table->capacity;
    } while (index != start_index);  // Full cycle, stop when we return to the start.

    *found = 0;  // Variable not found.
    return -1;   // Return invalid type.
}
