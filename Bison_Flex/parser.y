%{
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>

using namespace std;

// C++ Map for Symbol Table
map<string, string> symTable;

// Flex functions
extern int yylex();
extern char* yytext;
void yyerror(const char* s);
%}

%union {
    char* str;
}

%token INT FLOAT IF ELSE WHILE PRINT
%token <str> ID NUM RELOP

%type <str> Type Expr Term Factor

%%

Program: 
    StmtList { cout << "✅ Compilation Successful: No Syntax or Semantic errors!\n"; }
    ;

StmtList:
    Stmt StmtList
    | /* empty (epsilon) */
    ;

Stmt:
    DeclStmt
    | AssignStmt
    | IfStmt
    | WhileStmt
    | PrintStmt
    ;

DeclStmt:
    Type ID ';' {
        string varName = $2;
        string varType = $1;
        
        // Semantic Check 1: Duplicate Declaration
        if (symTable.find(varName) != symTable.end()) {
            cout << "❌ Semantic Error: Variable '" << varName << "' already declared\n";
            exit(1);
        }
        symTable[varName] = varType; // Add to symbol table
    }
    ;

Type:
    INT { $$ = (char*)"int"; }
    | FLOAT { $$ = (char*)"float"; }
    ;

AssignStmt:
    ID '=' Expr ';' {
        string varName = $1;
        string exprType = $3;
        
        // Semantic Check 2: Undeclared Variable
        if (symTable.find(varName) == symTable.end()) {
            cout << "❌ Semantic Error: Variable '" << varName << "' not declared\n";
            exit(1);
        }
        
        // Semantic Check 3: Type Mismatch
        string varType = symTable[varName];
        if (varType == "int" && exprType == "float") {
            cout << "❌ Type Error: Cannot assign float expression to int variable '" << varName << "'\n";
            exit(1);
        }
    }
    ;

IfStmt:
    IF '(' Cond ')' '{' StmtList '}' ElsePart
    ;

ElsePart:
    ELSE '{' StmtList '}'
    | /* empty */
    ;

WhileStmt:
    WHILE '(' Cond ')' '{' StmtList '}'
    ;

PrintStmt:
    PRINT '(' Expr ')' ';'
    ;

Cond:
    Expr RELOP Expr
    ;

Expr:
    Expr '+' Term { 
        if(string($1) == "float" || string($3) == "float") $$ = (char*)"float"; 
        else $$ = (char*)"int"; 
    }
    | Expr '-' Term { 
        if(string($1) == "float" || string($3) == "float") $$ = (char*)"float"; 
        else $$ = (char*)"int"; 
    }
    | Term { $$ = $1; }
    ;

Term:
    Term '*' Factor { 
        if(string($1) == "float" || string($3) == "float") $$ = (char*)"float"; 
        else $$ = (char*)"int"; 
    }
    | Term '/' Factor { 
        if(string($1) == "float" || string($3) == "float") $$ = (char*)"float"; 
        else $$ = (char*)"int"; 
    }
    | Factor { $$ = $1; }
    ;

Factor:
    ID {
        string varName = $1;
        if (symTable.find(varName) == symTable.end()) {
            cout << "❌ Semantic Error: Variable '" << varName << "' not declared in expression\n";
            exit(1);
        }
        // Bubble up the type from Symbol Table
        if (symTable[varName] == "float") $$ = (char*)"float";
        else $$ = (char*)"int";
    }
    | NUM {
        string val = $1;
        // If number contains '.', it's a float
        if (val.find('.') != string::npos) $$ = (char*)"float";
        else $$ = (char*)"int";
    }
    | '(' Expr ')' { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    cout << "❌ Syntax Error: " << s << " at token '" << yytext << "'\n";
    exit(1);
}

int main() {
    cout << "Enter your code (Press Ctrl+D to compile):\n";
    yyparse();
    return 0;
}