#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "quad.h"
#include "tables.h"
#include "codegen.h"
#include "utils.h"

// --- Global and Static Variables ---

// Counter for generated  unique temporary variable names.
static int temp_counter = 0;

// Counter for generated unique label names.
static int label_counter = 0;

// Pointer to the symbol table used to store the temporary variables during code generation.
static SymbolTable* symbol_table = NULL;

// The global instruction list (quad list) that holds the generated quads.
InstructionList* quad_list = NULL;

// --- Loop Stack Management ---

// Dynamic stack to manage nested loops and switch.
static char** loop_stack = NULL;

// Index of the top element in the loop stack.
static int loop_stack_top = -1;

// Current capacity of the loop stack.
static int loop_stack_capacity = 0;

// Initial capacity for the loop stack.
#define INITIAL_LOOP_STACK_CAPACITY 100

// --- Label Mapping Management ---

// Initial capacity for the label map (hash table).
static int label_map_capacity = 101;

// Hash table to map labels to their corresponding line numbers.
static LabelMapEntry** label_map = NULL;

// Number of labels currently in the map.
static int label_count = 0;

// --- Utility Functions ---

/**
 * Returns the current length of the quad list.
 */
int quad_list_length() {
    return quad_list ? quad_list->linenum : 0;
}

/**
 * Sets the line number associated with a given label in the label map.
 */
void set_label_line(const char* label, int line_number) {
    unsigned int index = hash_label(label);
    for (int i = 0; i < label_map_capacity; i++) {
        int try = (index + i) % label_map_capacity;
        if (label_map[try] && label_map[try]->in_use && strcmp(label_map[try]->label, label) == 0) {
            label_map[try]->line_number = line_number;
            fprintf(stderr, "[CHECK] LINE = %d\n", line_number);
            return;
        }
    }
    fprintf(stderr, "Error: Label %s not found to set line\n", label);
    exit(1);
}

/**
 * Hash function to compute the index of a label in the label map.
 */
unsigned int hash_label(const char* label) {
    unsigned int hash = 5381;
    int c;
    while ((c = *label++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % label_map_capacity;
}

/**
 * Creates an argument of type string.
 */
ArgValue make_string_arg(const char* str) {
    ArgValue arg;
    arg.type = ARG_TYPE_STRING;
    arg.str = strdup(str);  // Must be freed later.
    return arg;
}

/**
 * Creates an argument of type integer.
 */
ArgValue make_int_arg(int value) {
    ArgValue arg;
    arg.type = ARG_TYPE_INT;
    arg.number = value;
    return arg;
}

/**
 * Creates an empty (none) argument.
 */
ArgValue make_empty_arg() {
    ArgValue arg;
    arg.type = ARG_TYPE_NONE;
    return arg;
}

/**
 * Initializes the loop stack with an initial capacity.
 */
void init_loop_stack() {
    loop_stack_capacity = INITIAL_LOOP_STACK_CAPACITY;
    loop_stack = malloc(loop_stack_capacity * sizeof(char*));
    if (!loop_stack) {
        fprintf(stderr, "Error: Failed to allocate loop stack\n");
        exit(1);
    }
}

/**
 * Adds a new label to the label map with its corresponding line number.
 * Resizes the map if the load factor exceeds 80%.
 */
void map_label(const char* label, int line_number) {
    if (label_count >= label_map_capacity * 0.8) {
        resize_label_map(label_map_capacity * 2 + 1);
    }

    unsigned int index = hash_label(label);
    while (label_map[index]) {
        index = (index + 1) % label_map_capacity;
    }
    label_map[index] = malloc(sizeof(LabelMapEntry));
    label_map[index]->label = strdup(label);
    label_map[index]->line_number = line_number;
    label_map[index]->in_use = 1;
    label_count++;
}

/**
 * Resizes the label map (hash table) to a new capacity.
 * All existing entries are rehashed.
 */
void resize_label_map(int new_capacity) {
    LabelMapEntry** old_map = label_map;
    int old_capacity = label_map_capacity;

    label_map = calloc(new_capacity, sizeof(LabelMapEntry*));
    label_map_capacity = new_capacity;
    label_count = 0;

    for (int i = 0; i < old_capacity; i++) {
        if (old_map[i]) {
            map_label(old_map[i]->label, old_map[i]->line_number);
            free(old_map[i]->label);
            free(old_map[i]);
        }
    }
    free(old_map);
}

/**
 * Retrieves the line number associated with a given label.
 * If the label is not found, the program exits with an error.
 */
int get_label_line(const char* label) {
    unsigned int index = hash_label(label);
    for (int i = 0; i < label_map_capacity; i++) {
        int try = (index + i) % label_map_capacity;
        if (label_map[try] && label_map[try]->in_use && strcmp(label_map[try]->label, label) == 0) {
            return label_map[try]->line_number;
        }
    }
    fprintf(stderr, "Error: Label %s not found\n", label);
    exit(1);
}

/**
 * Frees all memory allocated for the label map.
 */
void free_label_map() {
    for (int i = 0; i < label_map_capacity; i++) {
        if (label_map[i] && label_map[i]->in_use) {
            free(label_map[i]->label);
            free(label_map[i]);
            label_map[i] = NULL;
        }
    }
    free(label_map);
    label_map = NULL;
}
// --- Loop Stack Management ---

// Pushes a loop label onto the loop stack.
// The stack grows dynamically if needed.
void push_loop_label(const char* label) {
    // Check if the stack is full, and if so, double its capacity.
    if (loop_stack_top + 1 >= loop_stack_capacity) {
        loop_stack_capacity *= 2;
        loop_stack = realloc(loop_stack, loop_stack_capacity * sizeof(char*));
        if (!loop_stack) {
            fprintf(stderr, "Error: Failed to reallocate loop stack\n");
            exit(1);
        }
    }
    // Push the label onto the stack by copying it.
    loop_stack[++loop_stack_top] = strdup(label);
}

// Pops the top loop label from the loop stack.
// Frees the memory used by the label.
void pop_loop_label() {
    if (loop_stack_top >= 0) {
        // Free the label at the top of the stack and decrease the stack pointer.
        free(loop_stack[loop_stack_top--]);
    }
}

// Returns the label at the top of the loop stack without removing it.
// Returns NULL if the stack is empty.
const char* current_loop_label() {
    return loop_stack_top >= 0 ? loop_stack[loop_stack_top] : NULL;
}

// Frees all memory allocated for the loop stack.
void free_loop_stack() {
    // Free each label string in the stack.
    for (int i = 0; i <= loop_stack_top; ++i) {
        free(loop_stack[i]);
    }
    // Free the stack array itself and reset the stack state.
    free(loop_stack);
    loop_stack = NULL;
    loop_stack_top = -1;
    loop_stack_capacity = 0;
}

// --- Temporary and Label Generators ---

// Creates and returns a new unique temporary variable name (t0, t1, ...).
const char* new_temp() {
    char* buf = malloc(16);  // Allocate space for the name.
    sprintf(buf, "t%d", temp_counter++);  // Format the temporary name.
    return buf;  
}

// Creates and returns a new unique label name (L0, L1, ...).
const char* new_label() {
    char* buf = malloc(16);  // Allocate space for the label name.
    sprintf(buf, "L%d", label_counter++);  // Format the label name.
    return buf; 
}

// --- Main Program Handler ---

// Processes the entire program: declarations, main statement block, and finalizes the quad list.
int handle_program(Program* prog) {
    // Initialize the quad instruction list.
    quad_list = create_instruction_list();
    fprintf(stderr, "[DEBUG] CHECK IF LINE 0 = %d\n", quad_list->linenum);

    fprintf(stderr, "[DEBUG] Starting handle_program()\n");

    // Initialize the loop label stack.
    init_loop_stack();

    // Create a new symbol table.
    symbol_table = create_table();

    // Initialize the label map for jump instructions.
    label_map = calloc(label_map_capacity, sizeof(LabelMapEntry*));

    // Process all variable declarations.
    handle_declarations(prog->declarations);

    // Process the main statement block.
    fprintf(stderr, "[DEBUG] Handling stmt block\n");
    handle_stmtblock(prog->stmt_block);

    // Append a HALT instruction at the end of the program.
    fprintf(stderr, "[DEBUG] Appending HALT quad\n");
    append_quad(quad_list, create_quad(HALT, make_empty_arg(), make_empty_arg(), NULL));

    // Patch all jump instructions to their correct target lines.
    fprintf(stderr, "[DEBUG] Patching jumps\n");
    patch_jumps();

    // Free the loop stack and other auxiliary structures.
    free_loop_stack();
    free_label_map();
    free_table(symbol_table);

    fprintf(stderr, "[DEBUG] Finished handle_program()\n");
    return 1;
}

// --- Declarations Handler ---

// Recursively processes all variable declarations in the program.
void handle_declarations(Declarations* decls) {
    if (decls == NULL || decls->type == EMPTY_DECS) return;

    fprintf(stderr, "[DEBUG] Handling declarations\n");

    // Process the current declaration.
    handle_declaration(decls->decl);

    // Recursively process the previous declarations.
    handle_declarations(decls->prev);
}
// --- Declaration Handler ---

// Processes a single declaration, adding each identifier in the declaration to the symbol table.
void handle_declaration(Declaration* dec) {
    fprintf(stderr, "[DEBUG] Handling declaration\n");

    // Loop through all identifiers in the declaration and add them to the symbol table.
    for (int i = 0; i < dec->idlist->count; ++i) {
        Identifier id = dec->idlist->items[i];
        table_add(symbol_table, id, dec->type);
    }
}

// --- Statement Block Handler ---

// Processes a block of statements.
void handle_stmtblock(StmtBlock* block) {
    fprintf(stderr, "[DEBUG] Entered handle_stmtblock()\n");
    fprintf(stderr, "[DEBUG] handle_stmtblock() ptr = %p\n", (void*)block);

    // Passes the statement list inside the block to the list handler.
    handle_stmtlist(block->stmtlist);
}

// --- Statement List Handler ---

// Recursively processes a list of statements.
void handle_stmtlist(StmtList* list) {
    fprintf(stderr, "[DEBUG] Entered handle_stmtlist()\n");
    static int depth = 0;
    depth++;
    fprintf(stderr, "[DEBUG] handle_stmtlist() depth = %d, ptr = %p\n", depth, (void*)list);

    // Base case: if the list is empty, stop recursion.
    if (list == NULL  || list->type == EMPTY_STMNT_LST) return;

    // Recursively process the previous statements.
    handle_stmtlist(list->prev);

    // Process the current statement.
    handle_statement(list->stmt);
}

// --- Single Statement Handler ---

// Processes a single statement according to its type.
void handle_statement(Statement* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_statement()\n");
    fprintf(stderr, "[DEBUG] stmt->type = %d\n", stmt->type);

    // Dispatch based on the statement type.
    switch (stmt->type) {
        case STMT_ASSIGN:
            handle_assignment(stmt->assign_stmt);
            break;

        case STMT_INPUT:
            fprintf(stderr, "[DEBUG] AAA\n");
            handle_input(stmt->input_stmt);
            break;

        case STMT_OUTPUT:
            handle_output(stmt->output_stmt);
            break;

        case STMT_IF:
            handle_if(stmt->if_stmt);
            break;

        case STMT_WHILE:
            handle_while(stmt->while_stmt);
            break;

        case STMT_BLOCK:
            handle_stmtblock(stmt->block);
            break;

        case STMT_BREAK: {
            // A break statement must appear inside a loop.
            const char* label = current_loop_label();
            if (!label) {
                fprintf(stderr, "Semantic Error: 'break' outside of loop or switch\n");
                exit(1);
            }
            append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), label));
            break;
        }

        case STMT_SWITCH:
            handle_switch(stmt->switch_stmt);
            break;
    }
}

// --- Assignment Handler ---

// Handles assignment statements, including type checking and conversions.
void handle_assignment(AssignmentStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_assignment()\n");

    int found;
    TypeEnum id_type = table_lookup(symbol_table, stmt->id, &found);
    if (!found) {
        fprintf(stderr, "Error: Undeclared variable %s\n", stmt->id);
        exit(1);
    }

    TypeEnum expr_type;
    const char* temp = handle_expression(stmt->expr, &expr_type);

    // Disallow assigning a float to an int variable.
    if (id_type == TYPE_INT && expr_type == TYPE_FLOAT) {
        fprintf(stderr, "Error: Cannot assign float to int variable %s\n", stmt->id);
        exit(1);
    }

    // Implicitly convert int to float if necessary.
    if (id_type == TYPE_FLOAT && expr_type == TYPE_INT) {
        const char* converted = new_temp();
        append_quad(quad_list, create_quad(ITOR, make_string_arg(temp), make_empty_arg(), converted));
        temp = converted;
    }

    // Choose the correct assignment opcode based on the variable type.
    OpCode op = (id_type == TYPE_INT) ? IASN : RASN;
    append_quad(quad_list, create_quad(op, make_string_arg(temp), make_empty_arg(), stmt->id));
}

// --- Input Handler ---

// Handles input statements, choosing the correct input operation based on the variable type.
void handle_input(InputStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_input()\n");

    int found;
    TypeEnum type = table_lookup(symbol_table, stmt->id, &found);
    if (!found) {
        fprintf(stderr, "Error: Undeclared variable %s\n", stmt->id);
        exit(1);
    }

    OpCode op = (type == TYPE_INT) ? IINP : RINP;
    append_quad(quad_list, create_quad(op, make_empty_arg(), make_empty_arg(), stmt->id));
}

// --- Output Handler ---

// Handles output statements, choosing the correct print operation based on the expression type.
void handle_output(OutputStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_output()\n");

    TypeEnum type;
    const char* temp = handle_expression(stmt->expr, &type);

    OpCode op = (type == TYPE_INT) ? IPRT : RPRT;
    append_quad(quad_list, create_quad(op, make_empty_arg(), make_empty_arg(), temp));
}

// --- If Statement Handler ---

// Handles if (and optional else) statements, including label management for jumps.
void handle_if(IfStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_if()\n");

    // Create labels for the else branch and the end of the if statement.
    const char* label_else = new_label();
    const char* label_end = stmt->else_stmt ? new_label() : NULL;

    // Pre-map the labels (to be updated later).
    map_label(label_end, -1);
    map_label(label_else, -1);

    // Handle the boolean expression (will jump to label_else if false).
    handle_boolexpr(stmt->cond, label_else);

    // Handle the 'then' part.
    handle_statement(stmt->if_stmt);

    if (stmt->else_stmt) {
        // Add unconditional jump to skip the else part.
        append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), label_end));
        fprintf(stderr, "[CHECK] QUAD LINE = %d\n", quad_list->linenum);

        // Mark the start of the else part.
        fprintf(stderr, "[CHECK] LINE IN IF = %d\n", quad_list_length());
        set_label_line(label_else, quad_list_length());

        // Handle the 'else' part.
        handle_statement(stmt->else_stmt);

        fprintf(stderr, "[CHECK] LINE IN IF = %d\n", quad_list_length());

        // Mark the end of the if-else statement.
        set_label_line(label_end, quad_list_length());

    } else {
        // If there is no else, set the else label to the current line (skip else block).
        set_label_line(label_else, quad_list_length());
    }
}

// --- While Statement Handler ---

// Handles while loops, generating labels and managing loop control flow.
void handle_while(WhileStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_while()\n");
    fprintf(stderr, "[CHECK] IN WHILE  stmt ptr: %p\n", (void*)stmt);

    const char* label_start = new_label(); // Label at the start of the loop
    const char* label_exit = new_label();  // Label at the end of the loop

    fprintf(stderr, "[DEBUG] AAA\n");

    map_label(label_start, -1); // Reserve start label

    set_label_line(label_start, quad_list_length()); // Mark loop start location
    fprintf(stderr, "[DEBUG] BBB\n");

    push_loop_label(label_exit); // Push exit label to manage 'break' statements

    handle_boolexpr(stmt->cond, label_exit); // Evaluate condition; if false, jump to exit

    fprintf(stderr, "[CHECK] IN WHILE  ptr: %p, type = %d\n", stmt->body, stmt->body->type);
    handle_statement(stmt->body); // Execute loop body

    // Jump back to start of the loop
    append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), label_start));

    map_label(label_exit, -1); // Reserve exit label
    set_label_line(label_exit, quad_list_length()); // Mark loop exit location
    fprintf(stderr, "[DEBUG] DDD\n");

    pop_loop_label(); // Clean up loop label stack
}

// --- Switch Statement Handler ---

// Handles switch-case structures, including the default case.
void handle_switch(SwitchStmt* stmt) {
    fprintf(stderr, "[DEBUG] Entered handle_switch()\n");

    TypeEnum type1;
    const char* expr_temp = handle_expression(stmt->expr, &type1);
    if (type1 != TYPE_INT) {
        semantic_error("switch expression must be of type int");
    }

    const char* end_label = new_label(); // Label to jump to after the switch block
    push_loop_label(end_label); // Support for 'break' inside the switch

    handle_caselist(stmt->cases, expr_temp, end_label); // Process each case

    // If there is a default block, process it
    if (stmt->default_stmtlist != NULL) {
        handle_stmtlist(stmt->default_stmtlist);
    }

    map_label(end_label, -1); // Reserve end label
    set_label_line(end_label, quad_list_length()); // Mark end of switch block

    pop_loop_label(); // Clean up loop label stack
}

// --- Case List Handler ---

// Recursively processes a list of cases in reverse order to maintain original case sequence.
void handle_caselist(CaseList* cases, const char* expr_temp, const char* end_label) {
    fprintf(stderr, "[DEBUG] Entered handle_caselist()\n");

    if (!cases || !cases->case1) return;

    // Process previous cases first to maintain order
    handle_caselist(cases->prev, expr_temp, end_label);

    // Then process the current case
    handle_case(cases->case1, expr_temp, end_label);
}

// --- Integer to String Utility ---

// Converts an integer to a string for comparison use in quads.
char* int_to_string(int value) {
    char* buffer = malloc(12);
    sprintf(buffer, "%d", value);
    return buffer;
}

// --- Single Case Handler ---

// Handles an individual case inside a switch statement.
void handle_case(Case* case_stmt, const char* expr_temp, const char* end_label) {
    fprintf(stderr, "[DEBUG] Entered case()\n");

    const char* skip_label = new_label(); // Label to skip the current case
    const char* temp_cmp = new_temp();    // Temporary variable for comparison

    const char* val_str = int_to_string(case_stmt->value);

    // Compare switch expression to case value
    append_quad(quad_list, create_quad(IEQL, make_string_arg(expr_temp), make_string_arg(val_str), temp_cmp));
    // If not equal, jump to skip label
    append_quad(quad_list, create_quad(JMPZ, make_string_arg(temp_cmp), make_empty_arg(), skip_label));

    // If equal, execute the case statements
    handle_stmtlist(case_stmt->stmts);

    // After the case, jump to the end of the switch
    append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), end_label));

    map_label(skip_label, -1); // Reserve skip label
    set_label_line(skip_label, quad_list_length()); // Mark skip location
}

// --- Expression Handler ---

// Handles arithmetic expressions, including type checking and implicit conversions.
const char* handle_expression(Expression* expr, TypeEnum* type) {
    fprintf(stderr, "[DEBUG] Entered handle_expression()\n");
    fprintf(stderr, "[CHECK] expr ptr: %p, expr->type = %d\n", (void*)expr, expr->type);

    // If the expression is a single term, process it directly
    if (expr->type == TERM_EXPR) {
        return handle_term(expr->only_term, type);
    }
    // If the expression is an addition or subtraction
    else if (expr->type == ADD_EXPR) {
        TypeEnum type1, type2;

        // Evaluate both sides of the expression
        const char* left = handle_expression(expr->add_expr.expr, &type1);
        const char* right = handle_term(expr->add_expr.term, &type2);

        // Perform type conversions if needed
        if (type1 == TYPE_INT && type2 == TYPE_FLOAT) {
            const char* conv = new_temp();
            append_quad(quad_list, create_quad(ITOR, make_string_arg(left), make_empty_arg(), conv));
            left = conv;
            type1 = TYPE_FLOAT;
        } else if (type1 == TYPE_FLOAT && type2 == TYPE_INT) {
            const char* conv = new_temp();
            append_quad(quad_list, create_quad(ITOR, make_string_arg(right), make_empty_arg(), conv));
            right = conv;
            type2 = TYPE_FLOAT;
        }

        // Determine the appropriate operation based on the types
        OpCode op;
        fprintf(stderr, "[DEBUG] AddOpType: %d\n", expr->add_expr.op);

        int is_float = (type1 == TYPE_FLOAT || type2 == TYPE_FLOAT);
        if (expr->add_expr.op == ADDOP_PLUS) {
            op = is_float ? RADD : IADD;
        } else if (expr->add_expr.op == ADDOP_MINUS) {
            op = is_float ? RSUB : ISUB;
        } else {
            fprintf(stderr, "[ERROR] Unknown AddOpType: %d\n", expr->add_expr.op);
            op = HALT; // Fallback error operation
        }

        const char* temp = new_temp();
        append_quad(quad_list, create_quad(op, make_string_arg(left), make_string_arg(right), temp));

        *type = is_float ? TYPE_FLOAT : TYPE_INT;
        return temp;
    }

    fprintf(stderr, "[ERROR] Unknown Expression type: %d\n", expr->type);
    exit(1);
}




// Handles the evaluation and code generation for a term node in the expression tree.
const char* handle_term(Term* term, TypeEnum* type) {
    fprintf(stderr, "[DEBUG] Entered handle_term()\n");

    if (term->type == FACTOR_TERM) {
        // Simple term consisting of a single factor; delegate to handle_factor.
        return handle_factor(term->factor, type);
    } else if (term->type == MUL_TERM) {
        TypeEnum type1, type2;
        // Recursively handle left sub-term and right factor, get their types and temporary results.
        const char* left = handle_term(term->multerm.ter, &type1);
        const char* right = handle_factor(term->multerm.factor, &type2);

        // Perform type promotion if one operand is int and the other float.
        if (type1 == TYPE_INT && type2 == TYPE_FLOAT) {
            const char* conv = new_temp();
            append_quad(quad_list, create_quad(ITOR, make_string_arg(left), make_empty_arg(), conv));
            left = conv;
            type1 = TYPE_FLOAT;  // Promote left operand type to float.
        } else if (type1 == TYPE_FLOAT && type2 == TYPE_INT) {
            const char* conv = new_temp();
            append_quad(quad_list, create_quad(ITOR, make_string_arg(right), make_empty_arg(), conv));
            right = conv;
            type2 = TYPE_FLOAT;  // Promote right operand type to float.
        }

        // Decide which opcode to use based on operator and operand types.
        OpCode op;
        int is_float = (type1 == TYPE_FLOAT || type2 == TYPE_FLOAT);
        if (term->multerm.op == MULOP_MUL)
            op = is_float ? RMLT : IMLT;  // Floating-point multiply or integer multiply
        else if (term->multerm.op == MULOP_DIV)
            op = is_float ? RDIV : IDIV;  // Floating-point divide or integer divide
        else {
            fprintf(stderr, "[ERROR] Unknown MULOP in term\n");
            exit(1);
        }

        // Create a new temporary to store the result of the operation.
        const char* temp = new_temp();
        append_quad(quad_list, create_quad(op, make_string_arg(left), make_string_arg(right), temp));
        *type = is_float ? TYPE_FLOAT : TYPE_INT;  // Result type depends on operand types.
        return temp;
    }

    fprintf(stderr, "[ERROR] Unknown term type\n");
    exit(1);
}

// Handles the evaluation and code generation for a factor node in the expression tree.
const char* handle_factor(Factor* factor, TypeEnum* type) {
    fprintf(stderr, "[DEBUG] Entered handle_factor()\n");

    switch (factor->type) {
        case FACTOR_NUM: {
            // Handle numeric literal (int or float).
            char buffer[64];
            const char* temp = new_temp();

            if (factor->num->type == TYPE_INT) {
                *type = TYPE_INT;
                snprintf(buffer, sizeof(buffer), "%d", factor->num->int_val);
                // Generate code to assign integer literal to temp.
                append_quad(quad_list, create_quad(IASN, make_string_arg(strdup(buffer)), make_empty_arg(), temp));
            } else if (factor->num->type == TYPE_FLOAT) {
                *type = TYPE_FLOAT;
                snprintf(buffer, sizeof(buffer), "%f", factor->num->double_val);
                // Generate code to assign float literal to temp.
                append_quad(quad_list, create_quad(RASN, make_string_arg(strdup(buffer)), make_empty_arg(), temp));
            } else {
                fprintf(stderr, "Error: Unknown literal type in FACTOR_NUM\n");
                exit(1);
            }

            return temp;
        }

        case FACTOR_ID: {
            // Lookup the identifier's type in the symbol table.
            int found;
            *type = table_lookup(symbol_table, factor->id, &found);
            if (!found) {
                fprintf(stderr, "Error: Undeclared variable %s\n", factor->id);
                exit(1);
            }
            // Return identifier name directly as it represents a variable.
            return factor->id;
        }

        case FACTOR_EXPR:
            // Factor is a nested expression; delegate to handle_expression.
            return handle_expression(factor->expr, type);

        case FACTOR_CAST: {
            // Handle explicit type cast of an expression.
            TypeEnum inner_type;
            const char* temp = handle_expression(factor->castexpr.expr, &inner_type);

            if (factor->castexpr.casttype == CAST_INT) {
                if (inner_type == TYPE_INT) {
                    // Cast to int from int is no-op.
                    *type = TYPE_INT;
                    return temp;
                }
                // Insert cast instruction from float to int.
                const char* result = new_temp();
                append_quad(quad_list, create_quad(RTOI, make_string_arg(temp), make_empty_arg(), result));
                *type = TYPE_INT;
                return result;
            }

            if (factor->castexpr.casttype == CAST_FLOAT) {
                if (inner_type == TYPE_FLOAT) {
                    // Cast to float from float is no-op.
                    *type = TYPE_FLOAT;
                    return temp;
                }
                // Insert cast instruction from int to float.
                const char* result = new_temp();
                append_quad(quad_list, create_quad(ITOR, make_string_arg(temp), make_empty_arg(), result));
                *type = TYPE_FLOAT;
                return result;
            }

            fprintf(stderr, "Error: Unknown cast type in FACTOR_CAST\n");
            exit(1);
        }

        default:
            fprintf(stderr, "Error: Unknown factor type\n");
            exit(1);
    }

    return NULL; // Never reached
}

// Handles boolean expressions and generates conditional jump instructions.
void handle_boolexpr(BoolExpr* expr, const char* false_label) {
    fprintf(stderr, "[DEBUG] Entered handle_boolexpr()\n");

    if (expr->type == BOOL_TERM) {
        // Simple boolean term, delegate to handle_boolterm.
        handle_boolterm(expr->right, false_label);
    } else {
        // Logical OR: if left side is true, jump to true_label skipping right side.
        const char* true_label = new_label();
        const char* check_other_cond = new_label();

        // Evaluate left side, jump to check_other_cond if false.
        handle_boolexpr(expr->boolor.left, check_other_cond);

        // If left side true, jump to true_label skipping right.
        append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), true_label));

        // Label to continue checking right condition if left was false.
        map_label(check_other_cond, -1);
        set_label_line(check_other_cond, quad_list_length());

        // Evaluate right side with the same false_label.
        handle_boolterm(expr->boolor.right, false_label);

        // Label marking the end of OR evaluation (true case).
        map_label(true_label, -1);
        set_label_line(true_label, quad_list_length());
    }
}

// Handles boolean terms (AND expressions) and generates conditional jump instructions.
void handle_boolterm(BoolTerm* term, const char* false_label) {
    fprintf(stderr, "[DEBUG] Entered handle_boolterm()\n");

    if (term->type == BOOL_FACTOR) {
        // Simple boolean factor, delegate to handle_boolfactor.
        handle_boolfactor(term->right, false_label);
    } else {
        // Logical AND: if left side is false, jump to false_label immediately.
        const char* next_label = new_label();

        // Evaluate left side with false_label for short-circuit.
        handle_boolterm(term->booland.left, false_label);

        // Label to continue evaluating right side only if left was true.
        map_label(next_label, -1);
        set_label_line(next_label, quad_list_length());

        // Evaluate right side with false_label.
        handle_boolfactor(term->booland.right, false_label);
    }
}

// Handles boolean factors (NOT expressions or relational expressions)
void handle_boolfactor(BoolFactor* factor, const char* false_label) {
    fprintf(stderr, "[DEBUG] Entered handle_boolfactor()\n");

    if (factor->type == BOOL_NOT) {
        // Handle logical NOT: invert the condition
        const char* true_label = new_label();

        // Evaluate the inner boolean expression with the true label
        handle_boolexpr(factor->expr, true_label);

        // If NOT expression is false, jump to false_label
        append_quad(quad_list, create_quad(JMP, make_empty_arg(), make_empty_arg(), false_label));

        // Mark the position of true_label
        map_label(true_label, -1);
        set_label_line(true_label, quad_list_length());
        return;
    }

    // Regular relational expression (e.g., a > b)
    TypeEnum left_type, right_type;

    // Evaluate both sides and get their types
    const char* left = handle_expression(factor->rel_expr.left, &left_type);
    const char* right = handle_expression(factor->rel_expr.right, &right_type);

    // Perform type casting if needed
    if (left_type == TYPE_INT && right_type == TYPE_FLOAT) {
        const char* conv = new_temp();
        append_quad(quad_list, create_quad(ITOR, make_string_arg(left), make_empty_arg(), conv));
        left = conv;
        left_type = TYPE_FLOAT;
    } else if (left_type == TYPE_FLOAT && right_type == TYPE_INT) {
        const char* conv = new_temp();
        append_quad(quad_list, create_quad(ITOR, make_string_arg(right), make_empty_arg(), conv));
        right = conv;
        right_type = TYPE_FLOAT;
    }

    RelOpType rel = factor->rel_expr.op;
    OpCode op;
    const char* temp = new_temp();

    // Handle inverted comparisons (like >= or <=)
    if (rel == RELOP_GTE || rel == RELOP_LTE) {
        // Map the relation to its inverse opcode
        op = map_relop_to_opcode(rel, left_type, right_type);

        // Compare left and right operands
        append_quad(quad_list, create_quad(op, make_string_arg(left), make_string_arg(right), temp));

        // Build (1 - temp) to invert the boolean result
        const char* inv = new_temp();
        append_quad(quad_list, create_quad(IASN, make_int_arg(1), make_empty_arg(), inv));

        const char* result = new_temp();
        append_quad(quad_list, create_quad(ISUB, make_string_arg(inv), make_string_arg(temp), result));

        // If result == 0, jump to false_label
        append_quad(quad_list, create_quad(JMPZ, make_string_arg(result), make_empty_arg(), false_label));
    } else {
        // Map the relational operator normally
        op = map_relop_to_opcode(rel, left_type, right_type);

        // Compare left and right operands
        append_quad(quad_list, create_quad(op, make_string_arg(left), make_string_arg(right), temp));

        // If comparison result is zero (false), jump to false_label
        append_quad(quad_list, create_quad(JMPZ, make_string_arg(temp), make_empty_arg(), false_label));
    }
}

// Maps relational operators to the correct opcode based on operand types
OpCode map_relop_to_opcode(RelOpType relop, TypeEnum left_type, TypeEnum right_type) {
    int is_float = (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT);

    switch (relop) {
        case RELOP_EQ:  return is_float ? REQL  : IEQL; // Equal
        case RELOP_NEQ: return is_float ? RNQL  : INQL; // Not equal
        case RELOP_LT:  return is_float ? RLSS  : ILSS; // Less than
        case RELOP_GT:  return is_float ? RGRT  : IGRT; // Greater than
        case RELOP_LTE: return is_float ? RGRT  : IGRT; // Inverted: a <= b --> a > b inverted later
        case RELOP_GTE: return is_float ? RLSS  : ILSS; // Inverted: a >= b --> a < b inverted later
        default:
            fprintf(stderr, "[ERROR] Unknown RelOpType: %d\n", relop);
            return HALT;
    }
}

// Patches jump instructions with correct line numbers
void patch_jumps() {
    Instruction* curr = quad_list->head;

    while (curr) {
        // Look for JMP or JMPZ that still hold label names
        if ((curr->op == JMP || curr->op == JMPZ) && curr->result && curr->result[0] == 'L') {
            int line = get_label_line(curr->result);

            // Replace the label with the actual line number
            char* line_str = malloc(16);
            sprintf(line_str, "%d", line);

            free(curr->result);
            curr->result = line_str;
        }
        curr = curr->next;
    }
}

// Getter for the global quad list
InstructionList* get_quad_list() {
    return quad_list;
}
