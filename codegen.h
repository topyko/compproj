#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "quad.h"

extern InstructionList* quad_list;  // The list of generated intermediate code instructions (quads).

// --- Helper Types ---

/**
 * Represents a mapping between a label and its corresponding instruction line number.
 */
typedef struct {
    char* label;        // The label name.
    int line_number;    // The corresponding line number in the quad list.
    int in_use;         // Indicates if the entry is active (1) or not (0).
} LabelMapEntry;

// --- Helper Functions for Label Mapping ---

/**
 * Maps a label to a specific line number.
 * @param label The label name.
 * @param line_number The instruction line number.
 */
void map_label(const char* label, int line_number);

/**
 * Retrieves the instruction line number for a given label.
 * @param label The label name.
 * @return The corresponding line number.
 */
int get_label_line(const char* label);

/**
 * Updates the instruction line number associated with a label.
 * @param label The label name.
 * @param line_number The new instruction line number.
 */
void set_label_line(const char* label, int line_number);

/**
 * Resizes the internal label map to support more labels.
 * @param new_capacity The new maximum capacity.
 */
void resize_label_map(int new_capacity);

/**
 * Frees all memory used by the label map.
 */
void free_label_map(void);

/**
 * Computes a hash value for a given label.
 * @param label The label name.
 * @return The hash index.
 */
unsigned int hash_label(const char* label);

// --- Loop Stack Management Functions ---

/**
 * Initializes the loop label stack.
 */
void init_loop_stack(void);

/**
 * Pushes a loop label onto the loop stack.
 * @param label The loop label.
 */
void push_loop_label(const char* label);

/**
 * Pops the top loop label from the loop stack.
 */
void pop_loop_label(void);

/**
 * Retrieves the current (top) loop label from the stack.
 * @return The current loop label.
 */
const char* current_loop_label(void);

/**
 * Frees the memory used by the loop stack.
 */
void free_loop_stack(void);

// --- Temporary Identifiers and Labels ---

/**
 * Generates a new temporary variable name.
 * @return The generated temporary variable name.
 */
const char* new_temp(void);

/**
 * Generates a new unique label name.
 * @return The generated label name.
 */
const char* new_label(void);

// --- AST Processing Functions ---

/**
 * Entry point for processing the entire program.
 * @param prog The program AST node.
 * @return The result status (typically 0 for success).
 */
int handle_program(Program* prog);

/**
 * Handles a list of variable declarations.
 * @param decls The declarations AST node.
 */
void handle_declarations(Declarations* decls);

/**
 * Handles a single variable declaration.
 * @param dec The declaration AST node.
 */
void handle_declaration(Declaration* dec);

/**
 * Handles a statement block.
 * @param block The statement block AST node.
 */
void handle_stmtblock(StmtBlock* block);

/**
 * Handles a list of statements.
 * @param list The statement list AST node.
 */
void handle_stmtlist(StmtList* list);

/**
 * Handles a single statement.
 * @param stmt The statement AST node.
 */
void handle_statement(Statement* stmt);

/**
 * Handles an assignment statement.
 * @param stmt The assignment statement AST node.
 */
void handle_assignment(AssignmentStmt* stmt);

/**
 * Handles an input statement.
 * @param stmt The input statement AST node.
 */
void handle_input(InputStmt* stmt);

/**
 * Handles an output statement.
 * @param stmt The output statement AST node.
 */
void handle_output(OutputStmt* stmt);

/**
 * Handles an if statement.
 * @param stmt The if statement AST node.
 */
void handle_if(IfStmt* stmt);

/**
 * Handles a while loop.
 * @param stmt The while statement AST node.
 */
void handle_while(WhileStmt* stmt);

/**
 * Handles a switch statement.
 * @param stmt The switch statement AST node.
 */
void handle_switch(SwitchStmt* stmt);

/**
 * Handles a list of cases in a switch statement.
 * @param cases The case list AST node.
 * @param expr_temp The temporary variable holding the switch expression's value.
 * @param end_label The label to jump to after the switch block.
 */
void handle_caselist(CaseList* cases, const char* expr_temp, const char* end_label);

/**
 * Handles a single case in a switch statement.
 * @param case_stmt The case AST node.
 * @param expr_temp The temporary variable holding the switch expression's value.
 * @param end_label The label to jump to after the switch block.
 */
void handle_case(Case* case_stmt, const char* expr_temp, const char* end_label);

/**
 * Handles an expression and generates the corresponding intermediate code.
 * @param expr The expression AST node.
 * @param type Output parameter for the expression type.
 * @return The temporary variable holding the result.
 */
const char* handle_expression(Expression* expr, TypeEnum* type);

/**
 * Handles a term (part of an expression).
 * @param term The term AST node.
 * @param type Output parameter for the term type.
 * @return The temporary variable holding the result.
 */
const char* handle_term(Term* term, TypeEnum* type);

/**
 * Handles a factor (part of a term).
 * @param factor The factor AST node.
 * @param type Output parameter for the factor type.
 * @return The temporary variable holding the result.
 */
const char* handle_factor(Factor* factor, TypeEnum* type);

/**
 * Handles a boolean expression, generating jump instructions for false conditions.
 * @param expr The boolean expression AST node.
 * @param false_label The label to jump to if the condition is false.
 */
void handle_boolexpr(BoolExpr* expr, const char* false_label);

/**
 * Handles a boolean term.
 * @param term The boolean term AST node.
 * @param false_label The label to jump to if the condition is false.
 */
void handle_boolterm(BoolTerm* term, const char* false_label);

/**
 * Handles a boolean factor.
 * @param factor The boolean factor AST node.
 * @param false_label The label to jump to if the condition is false.
 */
void handle_boolfactor(BoolFactor* factor, const char* false_label);

/**
 * Updates jump instructions with their correct target line numbers.
 */
void patch_jumps(void);

/**
 * Retrieves the current list of generated quads.
 * @return Pointer to the instruction list.
 */
InstructionList* get_quad_list(void);

// --- Additional Helper Functions ---

/**
 * Maps a relational operator to its corresponding intermediate code opcode.
 * @param relop The relational operator type.
 * @param left_type The type of the left operand.
 * @param right_type The type of the right operand.
 * @return The corresponding OpCode.
 */
OpCode map_relop_to_opcode(RelOpType relop, TypeEnum left_type, TypeEnum right_type);

/**
 * Returns the current number of instructions (quads) in the list.
 * @return The number of quads.
 */
int quad_list_length(void);

#endif // CODEGEN_H
