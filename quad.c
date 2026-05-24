#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quad.h"

//
// Creates a new quadruple (intermediate code instruction).
// Parameters:
// - op: The operation code (type of instruction)
// - arg1: The first argument (can be a string or an integer)
// - arg2: The second argument (can be a string or an integer)
// - result: The result variable (as a string), can be NULL
// Returns: A pointer to the newly created Instruction.
//
Instruction* create_quad(OpCode op, ArgValue arg1, ArgValue arg2, const char* result) {
    Instruction* q = (Instruction*)malloc(sizeof(Instruction));
    q->op = op;
    q->arg1 = arg1;
    q->arg2 = arg2;
    q->result = result ? strdup(result) : NULL; // Duplicate the result string if it exists
    q->next = NULL; // Initialize next pointer for the linked list
    return q;
}

//
// Creates an empty instruction list.
// Returns: A pointer to the newly created empty InstructionList.
//
InstructionList* create_instruction_list() {
    InstructionList* list = malloc(sizeof(InstructionList));
    list->head = NULL;   // List is initially empty
    list->tail = NULL;   // No last element yet
    list->linenum = 1;   // Start line numbering from 1
    return list;
}

//
// Appends a new instruction (quad) to the end of the instruction list.
// Parameters:
// - list: The instruction list
// - quad: The instruction to add
//
void append_quad(InstructionList* list, Instruction* quad) {
    if (!list->head) { // If list is empty
        list->head = list->tail = quad;
    } else { // If list already has elements
        list->tail->next = quad;
        list->tail = quad;
    }
    list->linenum += 1; // Increment the line number (instruction count)
}

//
// Frees memory for an argument if it is of type string.
// Parameters:
// - arg: The argument to free
//
void free_argvalue(ArgValue arg) {
    if (arg.type == ARG_TYPE_STRING && arg.str) { // Free only if string
        free(arg.str);
    }
    // No action needed for integer arguments
}

//
// Frees the entire instruction list from memory.
// Parameters:
// - list: The instruction list to free
//
void free_quad_list(InstructionList* list) {
    Instruction* curr = list->head;
    while (curr) { // Traverse the list
        Instruction* next = curr->next;
        free_argvalue(curr->arg1); // Free first argument if needed
        free_argvalue(curr->arg2); // Free second argument if needed
        free(curr->result); // Free the result string if allocated
        free(curr); // Free the instruction node itself
        curr = next;
    }
    list->head = list->tail = NULL; // Reset list pointers
}

//
// Prints a single argument to a file (used when printing instructions).
// Parameters:
// - f: The file to print to
// - arg: The argument to print
//
void print_arg(FILE* f, ArgValue arg) {
    if (arg.type == ARG_TYPE_STRING) {
        fprintf(f, "%s", arg.str); // Print string argument
    } else if (arg.type == ARG_TYPE_INT) {
        fprintf(f, "%d", arg.number); // Print integer argument
    } else {
        fprintf(f, "_"); // Print underscore if no argument
    }
}

//
// Prints a single instruction (quadruple) to a file in formatted style.
// Parameters:
// - f: The file to print to
// - q: The instruction to print
//
void print_quad_to_file(FILE* f, Instruction* q) {
    fprintf(f, "%s\t", opcode_to_string(q->op));               // Print operation
    fprintf(f, "%s\t", q->result ? q->result : "_");           // Print result or underscore
    print_arg(f, q->arg1); fprintf(f, "\t");                   // Print first argument
    print_arg(f, q->arg2); fprintf(f, "\n");                   // Print second argument and new line
}

//
// Converts an OpCode enum value to its string representation.
// Parameters:
// - op: The OpCode to convert
// Returns: The string name of the opcode.
//
const char* opcode_to_string(OpCode op) {
    switch (op) {
        case IASN: return "IASN";
        case IPRT: return "IPRT";
        case IINP: return "IINP";
        case IEQL: return "IEQL";
        case INQL: return "INQL";
        case ILSS: return "ILSS";
        case IGRT: return "IGRT";
        case IADD: return "IADD";
        case ISUB: return "ISUB";
        case IMLT: return "IMLT";
        case IDIV: return "IDIV";

        case RASN: return "RASN";
        case RPRT: return "RPRT";
        case RINP: return "RINP";
        case REQL: return "REQL";
        case RNQL: return "RNQL";
        case RLSS: return "RLSS";
        case RGRT: return "RGRT";
        case RADD: return "RADD";
        case RSUB: return "RSUB";
        case RMLT: return "RMLT";
        case RDIV: return "RDIV";

        case ITOR: return "ITOR";
        case RTOI: return "RTOI";

        case JMP: return "JMP";
        case JMPZ: return "JMPZ";

        case HALT: return "HALT";

        default: return "UNKNOWN"; // For unexpected opcodes
    }
}
