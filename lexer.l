%{
#include "parser.tab.h"
#include "ast.h"        

#include <string.h>
#include <stdlib.h>

int column = 1;
extern int yylineno;
%}

%%

[ \t]+              { column += yyleng; }
\n                  { yylineno++; column = 1; }
"//".*              { /* Ignore comment */}

"cast<int>"         { yylval.casttype = CAST_INT; return CAST; }
"cast<float>"       { yylval.casttype = CAST_FLOAT; return CAST; }

"int"               { return INT; }
"float"             { return FLOAT; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"switch"            { return SWITCH; }
"case"              { return CASE; }
"default"           { return DEFAULT; }
"input"             { return INPUT; }
"output"            { return OUTPUT; }
"break"             { return BREAK; }

"=="                { yylval.reltype = RELOP_EQ; return RELOP; }
"!="                { yylval.reltype = RELOP_NEQ; return RELOP; }
"<="                { yylval.reltype = RELOP_LTE; return RELOP; }
">="                { yylval.reltype = RELOP_GTE; return RELOP; }
"<"                 { yylval.reltype = RELOP_LT; return RELOP; }
">"                 { yylval.reltype = RELOP_GT; return RELOP; }

"+"                 { yylval.addoptype = ADDOP_PLUS; return ADDOP; }
"-"                 { yylval.addoptype = ADDOP_MINUS; return ADDOP; }
"*"                 { yylval.muloptype = MULOP_MUL; return MULOP; }
"/"                 { yylval.muloptype = MULOP_DIV; return MULOP; }

"||"                { return OR; }
"&&"                { return AND; }
"!"                 { return NOT; }

"="                 { return '='; }
"("                 { return '('; }
")"                 { return ')'; }
"{"                 { return '{'; }
"}"                 { return '}'; }
";"                 { return ';'; }
","                 { return ','; }
":"                 { return ':'; }
[0-9]+\.[0-9]+ {
    yylval.numlit.type = TYPE_FLOAT;
    yylval.numlit.double_val = atof(yytext);
    return NUM;
}

[0-9]+ {
    yylval.numlit.type = TYPE_INT;
    yylval.numlit.int_val = atoi(yytext);
    return NUM;
}


[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.id = strdup(yytext);
    return ID;
}
"/*"([^*]|\*[^/])*"*/"      { /* Ignore comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* Ignore comment */ }


. {
    fprintf(stderr, "Unknown character: %s at line %d\n", yytext, yylineno);
    exit(1);
}

%%

int yywrap(void) {
    return 1;
}
