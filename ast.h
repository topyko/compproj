#ifndef AST_H
#define AST_H

/**
 * This file defines the Abstract Syntax Tree (AST) nodes for the CPL language.
 * These structures represent the parsed program and will be used for further compilation phases,
 * such as semantic analysis and code generation to QUAD.
 */

// Forward declarations of main AST node types
typedef struct Statement Statement;
typedef struct StmtList StmtList;
typedef struct Case Case;
typedef struct Term Term;
typedef struct BoolExpr BoolExpr;
typedef struct BoolTerm BoolTerm;
typedef struct Declarations Declarations;

/**
 * Supported data types in CPL.
 */
typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } TypeEnum;

/**
 * Relational operators.
 */
typedef enum {
    RELOP_EQ, RELOP_NEQ, RELOP_LT, RELOP_GT,
    RELOP_GTE, RELOP_LTE
} RelOpType;

/**
 * Arithmetic addition operators.
 */
typedef enum {
    ADDOP_PLUS,
    ADDOP_MINUS
} AddOpType;

/**
 * Arithmetic multiplication operators.
 */
typedef enum {
    MULOP_MUL,
    MULOP_DIV
} MulOpType;

/**
 * Type casting options.
 */
typedef enum {
    CAST_INT,
    CAST_FLOAT
} CastType;

/**
 * Supported statement types.
 */
typedef enum {
    STMT_ASSIGN,
    STMT_INPUT,
    STMT_OUTPUT,
    STMT_IF,
    STMT_WHILE,
    STMT_SWITCH,
    STMT_BREAK,
    STMT_BLOCK
} StmtType;

/**
 * Identifier type (variable name).
 */
typedef char* Identifier;

/**
 * Numeric literal representation.
 */
typedef struct NumLiteral {
    TypeEnum type;
    union {
        double double_val;
        int int_val;
    };
} NumLiteral;

/**
 * List of identifiers (variable names).
 */
typedef struct {
    Identifier* items;
    int count;
} IdList;

/**
 * Single variable declaration.
 */
typedef struct Declaration {
    IdList* idlist;
    TypeEnum type;
} Declaration;

/**
 * List of variable declarations.
 */
typedef struct Declarations {
    enum { 
        NON_EMPTY_DECS, 
        EMPTY_DECS 
    } type;
    Declaration* decl;
    Declarations* prev;
} Declarations;

/**
 * Arithmetic expression.
 */
typedef struct Expression {
    enum {
        ADD_EXPR,
        TERM_EXPR
    } type;

    union {
        struct {
            struct Expression* expr;
            AddOpType op;
            Term* term;
        } add_expr;

        Term* only_term;
    };
} Expression;

/**
 * List of statements.
 */
typedef struct StmtList {
    enum { 
        NOT_EMPTY_STMNT_LST , 
        EMPTY_STMNT_LST 
    } type;
    Statement* stmt;
    StmtList* prev;
} StmtList;

/**
 * List of cases in a switch statement.
 */
typedef struct CaseList {
    enum { 
        NOT_EMPTY_CASE_LST , 
        EMPTY_CASE_LST 
    } type;
    StmtList* stmtlst;
    Case* case1;
    struct CaseList* prev;
} CaseList;

/**
 * Factor in an arithmetic expression: number, identifier, expression, or cast.
 */
typedef struct Factor {
    enum { 
        FACTOR_EXPR, 
        FACTOR_CAST, 
        FACTOR_ID, 
        FACTOR_NUM 
    } type;

    union {
        struct {
            CastType casttype;
            Expression* expr;
        } castexpr;

        Expression* expr;
        Identifier id;
        NumLiteral* num;
    };
} Factor;

/**
 * Term in an arithmetic expression: multiplication/division of factors.
 */
typedef struct Term {
    enum {
        MUL_TERM,
        FACTOR_TERM
    } type;
    union {
        struct {
            Term* ter;
            MulOpType op;
            Factor* factor;
        } multerm;
        Factor* factor;
    };
} Term;

/**
 * Boolean factor: NOT expression or relational expression.
 */
typedef struct BoolFactor {
    enum {
        BOOL_NOT,
        BOOL_REL
    } type;

    union {
        BoolExpr* expr;
        struct {
            Expression* left;
            RelOpType op;
            Expression* right;
        } rel_expr;
    };
} BoolFactor;

/**
 * Boolean term: AND operations between boolean factors.
 */
typedef struct BoolTerm {
    enum {
        BOOL_AND,
        BOOL_FACTOR
    } type;
    union {
        struct {
            BoolTerm* left;
            BoolFactor* right;
        } booland;
        BoolFactor* right;
    };
} BoolTerm;

/**
 * Boolean expression: OR operations between boolean terms.
 */
typedef struct BoolExpr {
    enum {
        BOOL_OR,
        BOOL_TERM
    } type;
    union {
        struct {
            BoolExpr* left;
            BoolTerm* right;
        } boolor;
        BoolTerm* right;
    };
} BoolExpr;

/**
 * Assignment statement.
 */
typedef struct AssignmentStmt {
    Identifier id;
    Expression* expr;
} AssignmentStmt;

/**
 * Input statement (read a variable).
 */
typedef struct InputStmt {
    Identifier id;
} InputStmt;

/**
 * Output statement (print expression).
 */
typedef struct OutputStmt {
    Expression* expr;
} OutputStmt;

/**
 * If-else statement.
 */
typedef struct IfStmt {
    BoolExpr* cond;
    Statement* if_stmt;
    Statement* else_stmt;
} IfStmt;

/**
 * While loop statement.
 */
typedef struct WhileStmt {
    BoolExpr* cond;
    Statement* body;
} WhileStmt;

/**
 * Single case in a switch statement.
 */
typedef struct Case {
    int value;
    StmtList* stmts;
} Case;

/**
 * Switch statement.
 */
typedef struct SwitchStmt {
    Expression* expr;
    CaseList* cases;
    StmtList* default_stmtlist;
} SwitchStmt;

/**
 * Block of statements.
 */
typedef struct StmtBlock {
    StmtList* stmtlist;
} StmtBlock;

/**
 * Statement structure - can be any supported statement type.
 */
struct Statement {
    StmtType type;
    union {
        AssignmentStmt* assign_stmt;
        InputStmt* input_stmt;
        OutputStmt* output_stmt;
        IfStmt* if_stmt;
        WhileStmt* while_stmt;
        SwitchStmt* switch_stmt;
        StmtBlock* block;
    };
};

/**
 * Program node: contains declarations and the main statement block.
 */
typedef struct Program {
    Declarations* declarations;
    StmtBlock* stmt_block;
} Program;

// Function declarations for creating AST nodes are defined here.
// These functions will be used by the parser to build the tree.

// ===== Expressions =====
Expression* create_expression(Expression* left, AddOpType op, Term* right);
Expression* create_expression_term(Term* term);

Term* create_term(Term* left, MulOpType op, Factor* right);
Term* create_term_factor(Factor* factor);

Factor* create_factor_expr(Expression* expr);
Factor* create_factor_cast(CastType cast, Expression* expr);
Factor* create_factor_id(Identifier id);
Factor* create_factor_num(NumLiteral* num);

// ===== Boolean Expressions =====
BoolExpr* create_boolexpr_or(BoolExpr* left, BoolTerm* right);
BoolExpr* create_boolexpr_term(BoolTerm* term);

BoolTerm* create_boolterm_and(BoolTerm* left, BoolFactor* right);
BoolTerm* create_boolterm_factor(BoolFactor* factor);

BoolFactor* create_boolfactor_not(BoolExpr* expr);
BoolFactor* create_boolfactor_rel(Expression* left, RelOpType op, Expression* right);

// ===== Statements =====
Statement* create_stmt_assign(AssignmentStmt* stmt);
Statement* create_stmt_input(InputStmt* stmt);
Statement* create_stmt_output(OutputStmt* stmt);
Statement* create_stmt_if(IfStmt* stmt);
Statement* create_stmt_while(WhileStmt* stmt);
Statement* create_stmt_switch(SwitchStmt* stmt);
Statement* create_stmt_stmtblock(StmtBlock* block);
Statement* create_stmt_break();

AssignmentStmt* create_assignment(Identifier id, Expression* expr);
InputStmt* create_input(Identifier id);
OutputStmt* create_output(Expression* expr);
IfStmt* create_if(BoolExpr* cond, Statement* then_stmt, Statement* else_stmt);
WhileStmt* create_while(BoolExpr* cond, Statement* body);
SwitchStmt* create_switch(Expression* expr, CaseList* cases, StmtList* default_stmtlist);

Case* create_case(int value, StmtList* stmts);
CaseList* create_case_list();
void case_list_add_case(CaseList* list, Case* kase);
void case_list_set_default(CaseList* list, StmtList* stmts);

// ===== Statement Blocks and Program =====
StmtList* create_stmtlist();
StmtList* stmtlist_add(StmtList* list, Statement* stmt);
StmtBlock* create_stmtblock(StmtList* list);

Declaration* create_declaration(IdList* idlist, TypeEnum type);
Declarations* create_declarations();
Declarations* declarations_add(Declarations* decls, Declaration* decl);

Program* create_program(Declarations* decls, StmtBlock* block);

// ===== Helpers =====
NumLiteral* create_num_literal(TypeEnum type, double val);
Identifier* create_identifier(const char* name);

// ===== Missing Functions for parser.y =====
IdList* create_idlist(Identifier id);
IdList* append_idlist(IdList* list, Identifier id);

AssignmentStmt* create_assignment_stmt(Identifier id, Expression* expr);
InputStmt* create_input_stmt(Identifier id);
OutputStmt* create_output_stmt(Expression* expr);
IfStmt* create_if_stmt(BoolExpr* cond, Statement* then_stmt, Statement* else_stmt);
WhileStmt* create_while_stmt(BoolExpr* cond, Statement* body);
SwitchStmt* create_switch_stmt(Expression* expr, CaseList* cases, StmtList* default_stmtlist);

CaseList* caselist_add(CaseList* list, int value, StmtList* stmts);

Expression* create_add_expr(Expression* left, AddOpType op, Term* right);
Expression* create_term_from_expr(Term* term);
Factor* create_factor_expression(Expression* expr);

#endif // AST_H
