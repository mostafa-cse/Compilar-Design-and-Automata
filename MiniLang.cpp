#include <bits/stdc++.h>
using namespace std;

// ==========================================
// 1. DATA STRUCTURES
// ==========================================
struct Token {
    string type;
    string value;
};

vector<Token> tokens;
int current_pos = 0;
map<string, string> symbolTable;   
string current_expr_type = "int";  

// ==========================================
// ERROR HANDLING HELPER
// ==========================================
void error(const string& msg) {
    cout << "" << msg << "\n";
    exit(1);
}

// ==========================================
// 2. LEXICAL ANALYZER
// ==========================================
bool isKeyword(const string& word) {
    vector<string> keywords = {"int", "float", "if", "else", "while", "print"};
    return find(keywords.begin(), keywords.end(), word) != keywords.end();
}

void tokenize(const string& code) {
    tokens.clear();
    int i = 0, n = code.length();

    while (i < n) {
        if (isspace(code[i])) { i++; continue; }

        if (isalpha(code[i])) {
            string word = "";
            while (i < n && isalnum(code[i])) { word += code[i]; i++; }
            if (isKeyword(word)) tokens.push_back({"keyword", word});
            else tokens.push_back({"id", word});
            continue;
        }

        if (isdigit(code[i])) {
            string num = "";
            while (i < n && (isdigit(code[i]) || code[i] == '.')) { num += code[i]; i++; }
            tokens.push_back({"num", num});
            continue;
        }

        if (i + 1 < n) {
            string twoCharOp = code.substr(i, 2);
            if (twoCharOp == "<=" || twoCharOp == ">=" || twoCharOp == "==" || twoCharOp == "!=") {
                tokens.push_back({"operator", twoCharOp});
                i += 2;
                continue;
            }
        }

        string opsAndDelims = "+-*/=<>[]{};()";
        if (opsAndDelims.find(code[i]) != string::npos) {
            string type = (string("[]{};()").find(code[i]) != string::npos) ? "delimiter" : "operator";
            tokens.push_back({type, string(1, code[i])});
            i++;
            continue;
        }
        
        i++; 
    }
}

void printTokens() {
    cout << "Token Output:\n";
    for (const auto& t : tokens) {
        if (t.type == "id") cout << "[id:" << t.value << "] ";
        else if (t.type == "num") cout << "[num:" << t.value << "] ";
        else cout << "[" << t.value << "] "; 
        
        if (t.value == ";" || t.value == "{" || t.value == "}") cout << "\n";
    }
    cout << "\n";
}

// ==========================================
// 3. PARSER HELPER FUNCTIONS
// ==========================================
Token peek() {
    if (current_pos < tokens.size()) return tokens[current_pos];
    return {"EOF", "EOF"};
}

Token consume() {
    Token t = peek();
    current_pos++;
    return t;
}

void match(const string& expected_value) {
    if (peek().value == expected_value) {
        consume();
    } else {
        error("Syntax Error: Expected '" + expected_value + "' but found '" + peek().value + "'");
    }
}

// ==========================================
// 4. SYNTAX & SEMANTIC ANALYZER (LL(1) Grammar)
// ==========================================
void StmtList();
void Stmt();
void DeclStmt();
void AssignStmt();
void IfStmt();
void ElsePart();
void WhileStmt();
void PrintStmt();
void Cond();
void RelOp();
void Expr();
void ExprPrime();
void Term();
void TermPrime();
void Factor();

void Program() {
    StmtList();
    if (peek().type == "EOF") {
        cout << "Compilation Successfull!\n";
    } else {
        error("Error: Unrecognized code at the end.");
    }
}

void StmtList() {
    if (peek().type != "EOF" && peek().value != "}") {
        Stmt();
        StmtList();
    }
}

void Stmt() {
    string val = peek().value;
    string type = peek().type;

    if (val == "int" || val == "float") DeclStmt();
    else if (type == "id") AssignStmt();
    else if (val == "if") IfStmt();
    else if (val == "while") WhileStmt();
    else if (val == "print") PrintStmt();
    else error("Syntax Error: Unexpected token '" + val + "'");
}

void DeclStmt() {
    string varType = consume().value; 
    
    if (peek().type != "id") error("Syntax Error: Expected identifier after type.");
    string varName = consume().value;
    
    if (symbolTable.count(varName)) {
        error("Semantic Error: Variable '" + varName + "' already declared");
    }
    symbolTable[varName] = varType; 
    
    if (peek().value == "=") {
        consume(); 
        current_expr_type = "int"; 
        Expr(); 
        if (varType == "int" && current_expr_type == "float") {
            error("Type Error: Cannot assign float to int during initialization");
        }
    }
    
    match(";"); 
}

void AssignStmt() {
    string varName = consume().value; 
    
    if (!symbolTable.count(varName)) {
        error("Semantic Error: Variable '" + varName + "' not declared");
    }
    string varType = symbolTable[varName];

    match("=");
    current_expr_type = "int"; 
    Expr();
    
    if (varType == "int" && current_expr_type == "float") {
        error("Type Error: Cannot assign float to int");
    }

    match(";");
}

void IfStmt() {
    match("if"); match("("); Cond(); match(")");
    match("{"); StmtList(); match("}");
    ElsePart();
}

void ElsePart() {
    if (peek().value == "else") {
        consume();
        match("{"); StmtList(); match("}");
    }
}

void WhileStmt() {
    match("while"); match("("); Cond(); match(")");
    match("{"); StmtList(); match("}");
}

void PrintStmt() {
    match("print"); match("("); Expr(); match(")"); match(";");
}

void Cond() {
    current_expr_type = "int"; Expr(); 
    RelOp(); 
    current_expr_type = "int"; Expr();
}

void RelOp() {
    string val = peek().value;
    if (val == "<" || val == ">" || val == "<=" || val == ">=" || val == "==" || val == "!=") {
        consume();
    } else {
        error("Syntax Error: Expected Relational Operator, found '" + val + "'");
    }
}

void Expr() { Term(); ExprPrime(); }

void ExprPrime() {
    if (peek().value == "+" || peek().value == "-") {
        consume(); Term(); ExprPrime();
    }
}

void Term() { Factor(); TermPrime(); }

void TermPrime() {
    if (peek().value == "*" || peek().value == "/") {
        consume(); Factor(); TermPrime();
    }
}

void Factor() {
    Token t = peek();
    
    if (t.type == "num") {
        if (t.value.find('.') != string::npos) current_expr_type = "float";
        consume();
    } 
    else if (t.type == "id") {
        if (!symbolTable.count(t.value)) {
            error("Semantic Error: Variable '" + t.value + "' not declared in expression");
        }
        if (symbolTable[t.value] == "float") current_expr_type = "float";
        consume();
    } 
    else if (t.value == "(") {
        match("("); Expr(); match(")");
    } 
    else {
        error("Syntax Error: Invalid expression, expected id, number, or '('");
    }
}

// ==========================================
// 5. TEST RUNNER
// ==========================================
void runCompiler(const string& code) {
    cout << code << endl;
    
    symbolTable.clear();
    current_expr_type = "int";
    
    tokenize(code);
    printTokens(); 
    
    current_pos = 0;
    cout << "Output:\n";
    if (!tokens.empty()) Program();
    cout << "\n";
}

int main() {
    string code = "", line;
    while (getline(cin, line)) {
        code += line;
    }
    runCompiler(code);

    return 0;
}