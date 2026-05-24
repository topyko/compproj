#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "utils.h"

// ===== Expression Constructors =====

/**
 * Creates an addition/subtraction expression composed of a previous expression, 
 * an operator, and a terminal.
 * 
 * @param expr Pointer to the previous expression.
 * @param op Operator of type AddOpType (addition or subtraction).
 * @param term Pointer to a terminal (Term).
 * @return Pointer to the newly created Expression.
 */
Expression* create_expression(Expression* expr, AddOpType op, Term* term) {
    Expression* expression = smalloc(sizeof(Expression)); // Allocate memory
    expression->add_expr.expr = expr; // Store the previous expression
    expression->add_expr.op = op; // Store the operator
    expression->add_expr.term = term; // Store the terminal
    expression->type = ADD_EXPR; // Set the type
    fprintf(stderr, "[CREATE] Expression* = %p, type = %d (ADD_EXPR)\n", (void*)expression, expression->type);
    return expression;
}

/**
 * Creates an expression consisting of a single terminal.
 * 
 * @param term Pointer to a terminal (Term).
 * @return Pointer to the newly created Expression.
 */
Expression* create_term_from_expr(Term* term) {
    Expression* expression = smalloc(sizeof(Expression)); // Allocate memory
    expression->only_term = term; // Store the terminal
    expression->type = TERM_EXPR; // Set the type
    fprintf(stderr, "[CREATE] Expression* = %p, type = %d (TERM_EXPR)\n", (void*)expression, expression->type);
    return expression;
}

// ===== Boolean Expression Constructors =====

/**
 * Creates a boolean expression consisting of a single boolean terminal.
 * 
 * @param term Pointer to a boolean terminal (BoolTerm).
 * @return Pointer to the newly created BoolExpr.
 */
BoolExpr* create_boolexpr_term(BoolTerm* term) {
    BoolExpr* expr = smalloc(sizeof(BoolExpr)); // Allocate memory
    expr->right = term; // Store the boolean terminal
    expr->type = BOOL_TERM; // Set the type
    fprintf(stderr, "[CREATE] BoolExpr* = %p\n", (void*)expr);
    return expr;
}

/**
 * Creates a boolean terminal consisting of a single boolean factor.
 * 
 * @param factor Pointer to a boolean factor (BoolFactor).
 * @return Pointer to the newly created BoolTerm.
 */
BoolTerm* create_boolterm_factor(BoolFactor* factor) {
    BoolTerm* term = smalloc(sizeof(BoolTerm)); // Allocate memory
    term->right = factor; // Store the boolean factor
    term->type = BOOL_FACTOR; // Set the type
    fprintf(stderr, "[CREATE] BoolTerm* = %p\n", (void*)term);
    return term;
}

// ===== Term Constructors =====

/**
 * Creates a multiplication/division term composed of a previous term, 
 * an operator, and a factor.
 * 
 * @param ter Pointer to the previous term.
 * @param op Operator of type MulOpType (multiplication or division).
 * @param factor Pointer to a factor (Factor).
 * @return Pointer to the newly created Term.
 */
Term* create_term(Term* ter, MulOpType op, Factor* factor) {
    Term* term = smalloc(sizeof(Term)); // Allocate memory
    term->multerm.ter = ter; // Store the previous term
    term->multerm.op = op; // Store the operator
    term->multerm.factor = factor; // Store the factor
    term->type = MUL_TERM; // Set the type
    fprintf(stderr, "[CREATE] Term* = %p, type = %d (MUL_TERM)\n", (void*)term, term->type);
    return term;
}

/**
 * Creates a term consisting of a single factor.
 * 
 * @param factor Pointer to a factor (Factor).
 * @return Pointer to the newly created Term.
 */
Term* create_term_factor(Factor* factor) {
    Term* term = smalloc(sizeof(Term)); // Allocate memory
    term->factor = factor; // Store the factor
    term->type = FACTOR_TERM; // Set the type
    fprintf(stderr, "[CREATE] Term* = %p, type = %d (FACTOR_TERM)\n", (void*)term, term->type);
    return term;
}

// ===== Factor Constructors =====

/**
 * Creates a factor consisting of a nested expression.
 * 
 * @param expr Pointer to an expression (Expression).
 * @return Pointer to the newly created Factor.
 */
Factor* create_factor_expr(Expression* expr) {
    Factor* factor = smalloc(sizeof(Factor)); // Allocate memory
    factor->type = FACTOR_EXPR; // Set the type
    factor->expr = expr; // Store the nested expression
    fprintf(stderr, "[CREATE] Factor* = %p, type = %d (FACTOR_EXPR)\n", (void*)factor, factor->type);
    return factor;
}


// Creates a Factor node of type cast expression.
Factor* create_factor_cast(CastType casttype1, Expression* expr1) {
    // Allocate memory for the Factor.
    Factor* factor = smalloc(sizeof(Factor));
    // Set the type of the Factor to cast.
    factor->type = FACTOR_CAST;
    // Assign the cast type.
    factor->castexpr.casttype = casttype1;
    // Assign the cast expression.
    factor->castexpr.expr = expr1;
    // Print debug information.
    fprintf(stderr, "[CREATE] Factor* = %p, type = %d (FACTOR_CAST)\n", (void*)factor, factor->type);
    return factor;
}

// Creates a Factor node of type identifier.
Factor* create_factor_id(Identifier id) {
    Factor* factor = smalloc(sizeof(Factor));
    factor->type = FACTOR_ID; // Set the type to identifier.
    factor->id = id; // Store the identifier.
    fprintf(stderr, "[CREATE] Factor* = %p, type = %d (FACTOR_ID)\n", (void*)factor, factor->type);
    return factor;
}

// Creates a Factor node of type number literal.
Factor* create_factor_num(NumLiteral* num) {
    Factor* factor = smalloc(sizeof(Factor));
    factor->type = FACTOR_NUM; // Set the type to number.
    factor->num = num; // Store the number literal.
    fprintf(stderr, "[CREATE] Factor* = %p, type = %d (FACTOR_NUM)\n", (void*)factor, factor->type);
    return factor;
}

// Creates a BoolExpr node representing logical OR.
BoolExpr* create_boolexpr_or(BoolExpr* left, BoolTerm* right) {
    BoolExpr* expr = smalloc(sizeof(BoolExpr));
    expr->boolor.left = left; // Set the left operand.
    expr->boolor.right = right; // Set the right operand.
    expr->type=BOOL_OR; // Set the expression type to OR.
    fprintf(stderr, "[CREATE] BoolExpr (OR)* = %p\n", (void*)expr);
    return expr;
}

// Creates a BoolTerm node representing logical AND.
BoolTerm* create_boolterm_and(BoolTerm* left, BoolFactor* right) {
    BoolTerm* term = smalloc(sizeof(BoolTerm));
    term->booland.left = left; // Set the left operand.
    term->booland.right = right; // Set the right operand.
    term->type=BOOL_AND; // Set the term type to AND.
    fprintf(stderr, "[CREATE] BoolTerm (AND)* = %p\n", (void*)term);
    return term;
}

// Creates a BoolFactor node representing logical NOT.
BoolFactor* create_boolfactor_not(BoolExpr* expr) {
    BoolFactor* factor = smalloc(sizeof(BoolFactor));
    factor->type = BOOL_NOT; // Set the factor type to NOT.
    factor->expr = expr; // Store the expression to negate.
    fprintf(stderr, "[CREATE] BoolFactor* = %p, type = %d (BOOL_NOT)\n", (void*)factor, factor->type);
    return factor;
}

// Wraps an identifier as a full expression.
Expression* wrap_id_as_expression(Identifier id) {
    // Create a factor from the ID, wrap it as a term, then as an expression.
    return create_term_from_expr(create_term_factor(create_factor_id(id)));
}

// Wraps a Factor as a full expression.
Expression* wrap_factor_as_expression(Factor* factor) {
    // Create a term from the factor and wrap it as an expression.
    Term* term = create_term_factor(factor);
    return create_term_from_expr(term);
}

// Ensures the expression is properly wrapped; wraps if necessary.
Expression* wrap_if_needed(Expression* expr) {
    // If the expression is NULL or not a valid type, wrap it.
    if (!expr || (expr->type != ADD_EXPR && expr->type != TERM_EXPR)) {
        fprintf(stderr, "[WRAP] Invalid Expression (ptr: %p), forcing wrap\n", (void*)expr);
        // Assume expr is actually a Factor, cast it and wrap.
        Factor* f = (Factor*)expr;
        return wrap_factor_as_expression(f);
    }
    // Return the original expression if valid.
    return expr;
}

// Creates a BoolFactor node representing a relational operation.
BoolFactor* create_boolfactor_rel(Expression* left, RelOpType op, Expression* right) {
    // Wrap the left and right expressions if needed.
    left = wrap_if_needed(left);
    right = wrap_if_needed(right);

    BoolFactor* factor = smalloc(sizeof(BoolFactor));
    factor->type = BOOL_REL; // Set the factor type to relational.
    factor->rel_expr.left = left; // Store the left expression.
    factor->rel_expr.op = op; // Store the relational operator.
    factor->rel_expr.right = right; // Store the right expression.

    fprintf(stderr, "[CREATE] BoolFactor* = %p, type = %d (BOOL_REL)\n", (void*)factor, factor->type);
    return factor;
}

// ================== Statements ==================

// Creates an assignment statement node.
AssignmentStmt* create_assignment_stmt(Identifier id, Expression* expr) {
    AssignmentStmt* stmt = smalloc(sizeof(AssignmentStmt));
    stmt->id = id; // Store the identifier.
    stmt->expr = expr; // Store the assigned expression.
    fprintf(stderr, "[CREATE] AssignmentStmt* = %p\n", (void*)stmt);
    return stmt;
}

// Creates an input statement node.
InputStmt* create_input_stmt(Identifier id) {
    InputStmt* stmt = smalloc(sizeof(InputStmt));
    stmt->id = id; // Store the identifier to input into.
    fprintf(stderr, "[CREATE] InputStmt* = %p\n", (void*)stmt);
    return stmt;
}

// Creates an output statement node.
OutputStmt* create_output_stmt(Expression* expr) {
    OutputStmt* stmt = smalloc(sizeof(OutputStmt));
    stmt->expr = expr; // Store the expression to output.
    fprintf(stderr, "[CREATE] OutputStmt* = %p\n", (void*)stmt);
    return stmt;
}
// Creates an If statement node.
IfStmt* create_if_stmt(BoolExpr* cond, Statement* if_stmt, Statement* else_stmt) {
    // Allocate memory for the IfStmt.
    IfStmt* stmt = smalloc(sizeof(IfStmt));
    // Store the boolean condition.
    stmt->cond = cond;
    // Store the 'if' branch statement.
    stmt->if_stmt = if_stmt;
    // Store the 'else' branch statement.
    stmt->else_stmt = else_stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] IfStmt* = %p\n", (void*)stmt);
    return stmt;
}

// Creates a While statement node.
WhileStmt* create_while_stmt(BoolExpr* cond, Statement* body) {
    // Allocate memory for the WhileStmt.
    WhileStmt* stmt = smalloc(sizeof(WhileStmt));
    // Store the boolean loop condition.
    stmt->cond = cond;
    // Store the loop body statement.
    stmt->body = body;
    // Print debug information for the WhileStmt and its components.
    fprintf(stderr, "[CREATE] WhileStmt* = %p\n", (void*)stmt);
    fprintf(stderr, "[CREATE] WhileStmt* ptr  stmt->cond  = %p\n", (void*)stmt->cond);
    fprintf(stderr, "[CREATE] WhileStmt* ptr  stmt->body  = %p\n", (void*)stmt->body);
    return stmt;
}

// Creates a Switch statement node.
SwitchStmt* create_switch_stmt(Expression* expr, CaseList* cases, StmtList* default_stmtlist) {
    // Allocate memory for the SwitchStmt.
    SwitchStmt* stmt = smalloc(sizeof(SwitchStmt));
    // Store the expression to switch on.
    stmt->expr = expr;
    // Store the list of cases.
    stmt->cases = cases;
    // Store the default statement list.
    stmt->default_stmtlist = default_stmtlist;
    // Print debug information.
    fprintf(stderr, "[CREATE] SwitchStmt* = %p\n", (void*)stmt);
    return stmt;
}

// Creates a general Statement node of type assignment.
Statement* create_stmt_assign(AssignmentStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to assignment.
    s->type = STMT_ASSIGN;
    // Store the assignment statement.
    s->assign_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (ASSIGN)* = %p\n", (void*)s);
    return s;
}

// Wraps a Factor as a Term node.
Term* create_term_from_factor(Factor* factor) {
    // Allocate memory for the Term.
    Term* term = smalloc(sizeof(Term));
    // Set the term type to factor term.
    term->type = FACTOR_TERM;
    // Store the factor inside the term.
    term->factor = factor;
    // Print debug information.
    fprintf(stderr, "[CREATE] Term* = %p (from Factor), type = %d (FACTOR_TERM)\n", (void*)term, term->type);
    return term;
}

// Wraps a Term as an Expression node.
Expression* create_expression_from_term(Term* term) {
    // Allocate memory for the Expression.
    Expression* expr = smalloc(sizeof(Expression));
    // Set the expression type to term expression.
    expr->type = TERM_EXPR;
    // Store the term inside the expression.
    expr->only_term = term;
    // Print debug information.
    fprintf(stderr, "[CREATE] Expression* = %p (from Term), type = %d (TERM_EXPR)\n", (void*)expr, expr->type);
    return expr;
}

// Wraps a Factor directly as an Expression (via Term).
Expression* create_expression_from_factor(Factor* f) {
    // Create a term from the factor.
    Term* t = create_term_from_factor(f);
    // Create an expression from the term.
    return create_expression_from_term(t);
}

// Creates a general Statement node of type input.
Statement* create_stmt_input(InputStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to input.
    s->type = STMT_INPUT;
    // Store the input statement.
    s->input_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (INPUT)* = %p\n", (void*)s);
    return s;
}

// Creates a general Statement node of type output.
Statement* create_stmt_output(OutputStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to output.
    s->type = STMT_OUTPUT;
    // Store the output statement.
    s->output_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (OUTPUT)* = %p\n", (void*)s);
    return s;
}

// Creates a general Statement node of type if.
Statement* create_stmt_if(IfStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to if.
    s->type = STMT_IF;
    // Store the if statement.
    s->if_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (IF)* = %p\n", (void*)s);
    return s;
}
// Creates a general Statement node of type while.
Statement* create_stmt_while(WhileStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to WHILE.
    s->type = STMT_WHILE;
    // Store the WhileStmt.
    s->while_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (WHILE)* = %p\n", (void*)s);
    return s;
}

// Creates a general Statement node of type switch.
Statement* create_stmt_switch(SwitchStmt* stmt) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to SWITCH.
    s->type = STMT_SWITCH;
    // Store the SwitchStmt.
    s->switch_stmt = stmt;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (SWITCH)* = %p\n", (void*)s);
    return s;
}

// Creates a general Statement node of type block (statement block).
Statement* create_stmt_stmtblock(StmtBlock* block) {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to BLOCK.
    s->type = STMT_BLOCK;
    // Store the statement block.
    s->block = block;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (BLOCK)* = %p\n", (void*)s);
    return s;
}

// Creates a general Statement node of type break.
Statement* create_stmt_break() {
    // Allocate memory for the Statement.
    Statement* s = smalloc(sizeof(Statement));
    // Set the statement type to BREAK.
    s->type = STMT_BREAK;
    // Print debug information.
    fprintf(stderr, "[CREATE] Statement (BREAK)* = %p\n", (void*)s);
    return s;
}

// Creates an empty statement list.
StmtList* create_stmtlist() {
    // Allocate memory for the statement list.
    StmtList* new_LST = smalloc(sizeof(StmtList));
    // Initialize as an empty list.
    new_LST->stmt = NULL;
    new_LST->prev = NULL;
    new_LST->type = EMPTY_STMNT_LST;
    // Return the empty list.
    return new_LST;
}

// Adds a statement to a statement list.
StmtList* stmtlist_add(StmtList* list, Statement* stmt) {
    // Print debug information.
    fprintf(stderr, "[DEBUG] inserting into statment list\n");
    // Allocate memory for the new list node.
    StmtList* new_node = smalloc(sizeof(StmtList));
    // Set the node type to not empty.
    new_node->type = NOT_EMPTY_STMNT_LST;
    // Store the current statement.
    new_node->stmt = stmt;
    // Link to the previous list.
    new_node->prev = list;
    return new_node;
}

// Creates a statement block from a statement list.
StmtBlock* create_stmtblock(StmtList* list) {
    // Allocate memory for the statement block.
    StmtBlock* block = smalloc(sizeof(StmtBlock));
    // Store the statement list inside the block.
    block->stmtlist = list;
    return block;
}

// Creates a case node for a switch statement.
Case* create_case(int value, StmtList* stmts) {
    // Allocate memory for the case.
    Case* kase = smalloc(sizeof(Case));
    // Store the case value.
    kase->value = value;
    // Store the statement list associated with this case.
    kase->stmts = stmts;
    return kase;
}

// Creates an empty case list.
CaseList* create_case_list() {
    // Allocate memory for the case list.
    CaseList* list = smalloc(sizeof(CaseList));
    // Initialize as empty.
    list->case1 = NULL;
    list->prev = NULL;
    return list;
}

// Adds a case to the case list.
CaseList* caselist_add(CaseList* list, int value, StmtList* stmtlst1) {
    // Allocate memory for the new case list node.
    CaseList* new_node = malloc(sizeof(CaseList));
    // Allocate memory for the new case.
    new_node->case1 = malloc(sizeof(Case));
    // Store the case value.
    new_node->case1->value = value;
    // Store the statement list for this case.
    new_node->case1->stmts = stmtlst1;
    // Link to the previous list.
    new_node->prev = list;
    return new_node;
}

// Creates an empty declarations list.
Declarations* create_declarations() {
    // Allocate memory for the declarations list.
    Declarations* decls = smalloc(sizeof(Declarations));
    // Initialize as empty.
    decls->type = EMPTY_DECS;
    decls->decl = NULL;
    decls->prev = NULL;
    return decls;
}

// Adds a declaration to the declarations list.
Declarations* declarations_add(Declarations* list, Declaration* decl) {
    // Allocate memory for the new declarations node.
    Declarations* new_node = smalloc(sizeof(Declarations));
    // Set the type to non-empty.
    new_node->type = NON_EMPTY_DECS;
    // Store the declaration.
    new_node->decl = decl;
    // Link to the previous list.
    new_node->prev = list;
    return new_node;
}

// Creates a Program node from declarations and a statement block.
Program* create_program(Declarations* decls, StmtBlock* block) {
    // Allocate memory for the program.
    Program* prog = smalloc(sizeof(Program));
    // Store the declarations.
    prog->declarations = decls;
    // Store the main statement block.
    prog->stmt_block = block;
    return prog;
}

// Creates a numeric literal node, which can represent either an integer or a double.
NumLiteral* create_num_literal(TypeEnum type, double val) {
    // Allocate memory for the NumLiteral.
    NumLiteral* num = smalloc(sizeof(NumLiteral));
    // Store the type (TYPE_INT or TYPE_DOUBLE).
    num->type = type;
    // If the type is integer, cast and store the value as int.
    if (type == TYPE_INT)
        num->int_val = (int)val;
    else
        // Otherwise, store the value as double.
        num->double_val = val;
    return num;
}

// Creates a new identifier list containing a single identifier.
IdList* create_idlist(Identifier id) {
    // Allocate memory for the IdList structure.
    IdList* list = malloc(sizeof(IdList));
    // Allocate memory to store one identifier.
    list->items = malloc(sizeof(Identifier));
    // Copy the provided identifier into the list (deep copy using strdup).
    list->items[0] = strdup(id);
    // Initialize the count to 1 since there's one identifier.
    list->count = 1;
    return list;
}

// Appends a new identifier to an existing IdList.
IdList* append_idlist(IdList* list, Identifier id) {
    // Reallocate memory to store one more identifier.
    list->items = realloc(list->items, sizeof(Identifier) * (list->count + 1));
    // Add the new identifier to the end of the list (deep copy using strdup).
    list->items[list->count++] = strdup(id);
    return list;
}

// Creates a declaration node with a list of identifiers and their type.
Declaration* create_declaration(IdList* idlist, TypeEnum type) {
    // Allocate memory for the Declaration structure.
    Declaration* decl = malloc(sizeof(Declaration));
    // Store the list of identifiers in the declaration.
    decl->idlist = idlist;
    // Store the type associated with these identifiers.
    decl->type = type;
    return decl;
}

// Creates a break statement node.
Statement* create_break() {
    // Allocate memory for the Statement structure.
    Statement* stmt = malloc(sizeof(Statement));
    // Set the statement type to BREAK.
    stmt->type = STMT_BREAK;
    return stmt;
}
