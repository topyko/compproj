// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>  // For size_t
#include "quad.h"    // For InstructionList and print_quad_to_file

/**
 * Allocates memory safely.
 * If allocation fails, the program will terminate with an error message.
 * @param size The size in bytes to allocate.
 * @return Pointer to the allocated memory.
 */
void* smalloc(size_t size);

/**
 * Writes the list of quads (instructions) to a file.
 * @param filename The name of the output file.
 * @param quad_list The list of instructions to write.
 * @return 0 on success, non-zero on failure.
 */
int write_quads_to_file(const char* filename, InstructionList* quad_list);

/**
 * Prints a semantic error message and terminates the program.
 * @param message The error message to display.
 */
void semantic_error(const char* message);

#endif