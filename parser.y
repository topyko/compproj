%code requires {
    #include "ast.h"
}
%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"  

extern int yylineno; 
int yylex(void);
void yyerror(const char *s);
Program* ast_root;
%}

%union {
    int intval;                  
    float floatval;             
    int typeval;           
    AddOpType addoptype;         
    MulOpType muloptype;        
    RelOpType reltype;    
    CastType casttype;      
    Program* program;
    NumLiteral numlit;  

    Declarations* declarations;
    Declaration* declaration;
    StmtBlock* stmtblock;
    Statement* statement;
    AssignmentStmt* assignmentstmt;
    InputStmt* inputstmt;
    OutputStmt* outputstmt;
    IfStmt* ifstmt;
    WhileStmt* whilestmt;
    SwitchStmt* switchstmt;
    StmtList* stmtlist;
    Expression* expression;
    Term* term;
    Factor* factor;
    BoolExpr* boolexpr;
    BoolTerm* boolterm;
    BoolFactor* boolfactor;
    Identifier id;  
    IdList* idlist;
    CaseList* caselist;       
    TypeEnum type_enum;  
    
}


%token <id> ID
%token <numlit> NUM
%token <addoptype> ADDOP
%token <muloptype> MULOP
%token <reltype> RELOP
%token IF ELSE WHILE SWITCH CASE DEFAULT BREAK
%token INPUT OUTPUT FLOAT INT
%token AND OR NOT ASSIGN
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COLON COMMA 
%token <typeval> CAST
%type <typeval> type
%type <program> program
%type <declarations> declarations
%type <declaration> declaration
%type <stmtblock> stmt_block
%type <statement> stmt
%type <stmtlist> stmtlist
%type <assignmentstmt> assignment_stmt
%type <inputstmt> input_stmt
%type <outputstmt> output_stmt
%type <ifstmt> if_stmt
%type <whilestmt> while_stmt
%type <switchstmt> switch_stmt
%type <expression> expression
%type <term> term
%type <factor> factor
%type <boolexpr> boolexpr
%type <boolterm> boolterm
%type <boolfactor> boolfactor
%type <idlist> idlist
%type <caselist> caselist
%type <statement> break_stmt

%%

program:
    declarations stmt_block { $$ = create_program($1, $2); ast_root = $$; }
;

declarations:
    declarations declaration { $$ = declarations_add($1, $2); }
  | declaration              { $$ = declarations_add(create_declarations(), $1); }
  | /* empty */              { $$ = create_declarations(); }
;

declaration:
    idlist ':' type ';' { $$ = create_declaration($1, $3); }
  | error ';' {
        fprintf(stderr, "Syntax error in declaration at line %d\n", yylineno);
        yyerrok;
        yyclearin;
        $$ = NULL; 
    }
;


type:
      INT   { $$ = TYPE_INT; }
    | FLOAT { $$ = TYPE_FLOAT; }
  ;

idlist: ID              { $$ = create_idlist($1); }
      | idlist ',' ID   { $$ = append_idlist($1, $3); }
;
stmt:
    assignment_stmt { $$ = create_stmt_assign($1); }
  | input_stmt      { $$ = create_stmt_input($1); }
  | output_stmt     { $$ = create_stmt_output($1); }
  | if_stmt         { $$ = create_stmt_if($1); }
  | while_stmt      { $$ = create_stmt_while($1); }
  | switch_stmt     { $$ = create_stmt_switch($1); }
  | break_stmt      { $$ = (Statement*)$1; }
  | stmt_block      { $$ = create_stmt_stmtblock($1); }
  | error ';' {
        fprintf(stderr, "Syntax error in statement at line %d (skipping to ';')\n", yylineno);
        yyerrok;
        yyclearin;
        $$ = NULL;
    }
;

assignment_stmt:
    ID '=' expression ';' { $$ = create_assignment_stmt($1, $3); }

input_stmt:
    INPUT '(' ID ')' ';' { $$ = create_input_stmt($3); }

output_stmt:
    OUTPUT '(' expression ')' ';' { $$ = create_output_stmt($3); }
;
if_stmt:
    IF '(' boolexpr ')' stmt ELSE stmt { $$ = create_if_stmt($3, $5, $7); }
;
while_stmt:
    WHILE '(' boolexpr ')' stmt { $$ = create_while_stmt($3, $5); }
;

switch_stmt:
    SWITCH '(' expression ')' '{' caselist DEFAULT ':' stmtlist '}' { $$ = create_switch_stmt($3, $6, $9); }
;


break_stmt:
    BREAK ';' { $$ = create_stmt_break(); }
;

stmt_block:
    '{' stmtlist '}' { $$ = create_stmtblock($2); }
  | '{' error '}' {
        fprintf(stderr, "Syntax error in block at line %d\n", yylineno);
        yyerrok;
        yyclearin;
        $$ = NULL; 
    }
;

stmtlist:
    stmtlist stmt {
        if ($2 != NULL)
            $$ = stmtlist_add($1, $2);
        else
            $$ = $1;
    }
  | /* empty */ { $$ = create_stmtlist(); }
;

caselist:
    caselist CASE NUM ':' stmtlist { $$ = caselist_add($1, $3.int_val, $5); }
  | /* empty */ { $$ = create_case_list(); }
;
boolexpr:
    boolexpr OR boolterm          { $$ = create_boolexpr_or($1, $3); }
  | boolterm                      { $$ = create_boolexpr_term($1); }
  | error                         {
        fprintf(stderr, "Syntax error in condition at line %d\n", yylineno);
        yyerrok;
        $$ = NULL;
    }
;

boolterm:
    boolterm AND boolfactor { $$ = create_boolterm_and($1, $3); }
  |  boolfactor             { $$ = create_boolterm_factor($1); }
;

boolfactor:
    NOT '(' boolexpr ')' { $$ = create_boolfactor_not($3); }
  | expression RELOP expression { $$ = create_boolfactor_rel($1, $2, $3); }
;

expression:
    expression ADDOP term         { $$ = create_expression($1, $2, $3); }
  | term                          { $$ = create_term_from_expr($1); }
  | error                         {
        fprintf(stderr, "Syntax error in expression at line %d\n", yylineno);
        yyerrok;
        $$ = NULL;
    }
;

term:
    term MULOP factor { $$ = create_term($1, $2, $3); }
  | factor { $$ = create_term_factor($1); }
;

factor:
    '(' expression ')' { $$ = create_factor_expr($2); }
  | CAST '(' expression ')' { $$ = create_factor_cast($1, $3); }
  | ID { $$ = create_factor_id($1); }
  | NUM {
      $$ = create_factor_num(create_num_literal($1.type,
               $1.type == TYPE_INT ? $1.int_val : $1.double_val));
  }
;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parser error at line %d: %s\n", yylineno, s);
}
