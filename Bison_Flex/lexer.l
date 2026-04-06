%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap

%%
[ \t\n]+                { /* Ignore whitespace and newlines */ }

"int"                   { return INT; }
"float"                 { return FLOAT; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"print"                 { return PRINT; }

"<="|">="|"=="|"!="|"<"|">" { yylval.str = strdup(yytext); return RELOP; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return ID; }
[0-9]+(\.[0-9]+)?       { yylval.str = strdup(yytext); return NUM; }

"="                     { return '='; }
";"                     { return ';'; }
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }

.                       { printf("❌ Lexical Error: Unknown character %s\n", yytext); exit(1); }
%%