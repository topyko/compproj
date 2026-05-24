#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

/**
 * Allocates zero-initialized memory safely.
 * If the allocation fails, the program will print an error and exit.
 * @param size The size in bytes to allocate.
 * @return Pointer to the allocated memory block.
 */
void* smalloc(size_t size) {
    void* p = calloc(1, size);  // Allocate and zero-initialize memory
    if (!p) {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }
    return p;
}

/**
 * Writes a list of quads (intermediate instructions) to a file.
 * @param filename The name of the file to write to.
 * @param quad_list The list of instructions to write.
 * @return Returns 1 on success, 0 on failure to open the file.
 */
int write_quads_to_file(const char* filename, InstructionList* quad_list) {
    FILE* f = fopen(filename, "w");
    if (!f) {   // Handle file opening failure
        perror("Failed to open output file");
        return 0;
    }

    Instruction* curr = quad_list->head;
    while (curr != NULL) {
        print_quad_to_file(f, curr);  // Write each instruction to the file
        curr = curr->next; 
    }

    fclose(f);
    return 1;  // Success
}

/**
 * Prints a semantic error message to stderr and terminates the program.
 * @param message The error message to display.
 */
void semantic_error(const char* message) {
    fprintf(stderr, "Semantic Error: %s\n", message);
    exit(1);
}
