#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>

//
// OpCode: Enum representing the different types of operations (instructions)
// that can exist in the intermediate code (quadruples).
//
typedef enum {
    // Integer operations
    IASN, IPRT, IINP, IEQL, INQL, ILSS, IGRT, IADD, ISUB, IMLT, IDIV,
    // Real (floating point) operations
    RASN, RPRT, RINP, REQL, RNQL, RLSS, RGRT, RADD, RSUB, RMLT, RDIV,
    // Type conversions
    ITOR,  // Integer to Real
    RTOI,  // Real to Integer
    // Control flow operations
    JMP,   // Unconditional jump
    JMPZ,  // Jump if zero
    // Program termination
    HALT   // Stop execution
} OpCode;

//
// ArgType: Enum to indicate the type of argument a quadruple can have.
//
typedef enum {
    ARG_TYPE_NONE,    // No argument
    ARG_TYPE_STRING,  // Argument is a string (usually variable names)
    ARG_TYPE_INT      // Argument is an integer constant
} ArgType;

//
// ArgValue: Structure representing a value passed as an argument in a quad.
// The argument can be either a string (variable name) or an integer.
//
typedef struct {
    ArgType type;  // Type of the argument (none, string, int)
    union {
        char* str;   // If the argument is a string
        int number;  // If the argument is an integer
    };
} ArgValue;

//
// Instruction: Structure representing a single intermediate code instruction (quad).
//
typedef struct Instruction {
    OpCode op;         // Operation code (what the instruction does)
    ArgValue arg1;     // First argument
    ArgValue arg2;     // Second argument
    char* result;      // Result (usually the target variable name)
    struct Instruction* next;  // Pointer to the next instruction in the list (linked list)
} Instruction;

//
// InstructionList: Structure representing a list of instructions (a sequence of quads).
//
typedef struct {
    Instruction* head;  // Pointer to the first instruction
    Instruction* tail;  // Pointer to the last instruction (for efficient appending)
    int linenum;        // Current line number (useful for tracking instruction order)
} InstructionList;

//
// Function declarations:
// These functions handle the creation, management, printing, and destruction of the instruction list.
//

// Creates a new quadruple (instruction) with the given operation and arguments.
Instruction* create_quad(OpCode op, ArgValue arg1, ArgValue arg2, const char* result);

// Adds a quadruple to the end of the instruction list.
void append_quad(InstructionList* list, Instruction* quad);

// Frees the entire instruction list and all its nodes from memory.
void free_quad_list(InstructionList* list);

// Prints all instructions (quadruples) in the list to the console.
void print_quads(InstructionList* list);

// Prints a single instruction to a file.
void print_quad_to_file(FILE* f, Instruction* instr);

// Converts an OpCode to its string representation (for printing).
const char* opcode_to_string(OpCode op);

// Creates and initializes an empty instruction list.
InstructionList* create_instruction_list();

#endif