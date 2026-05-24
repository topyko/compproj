#ifndef TABLES_H
#define TABLES_H

#include "ast.h"  // Contains the definition of TypeEnum

// Represents a variable in the symbol table.
typedef struct Var {
    Identifier name;  // The variable's name.
    TypeEnum type;    // The variable's type (e.g., integer, real).
} Var;

// Symbol table structure for storing variables and their types.
typedef struct SymbolTable {
    Var* entries;     // Dynamic array holding the variables.
    int* used;        // Array indicating whether each slot is used (1) or free (0).
    int size;         // The current number of variables stored in the table.
    int capacity;     // The maximum number of variables the table can currently hold.
} SymbolTable;

// Creates and returns a new, empty symbol table.
// Returns: A pointer to the created SymbolTable.
SymbolTable* create_table();

// Frees all memory associated with the given symbol table, including the variables.
void free_table(SymbolTable* table);

// Adds a new variable to the symbol table.
// Parameters:
// - table: The symbol table to which the variable will be added.
// - name: The name of the new variable.
// - type: The type of the new variable.
void table_add(SymbolTable* table, Identifier name, TypeEnum type);

// Searches for a variable in the symbol table.
// Parameters:
// - table: The symbol table to search in.
// - name: The name of the variable to look up.
// - found: Pointer to an integer that will be set to 1 if the variable is found, or 0 otherwise.
// Returns: The type of the variable if found, or an undefined value if not found.
TypeEnum table_lookup(SymbolTable* table, Identifier name, int* found);

#endif
