#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cctype>

using namespace std;

// ==========================================
// 1. DATA STRUCTURES
// ==========================================
struct Token {
    string type;
    string value;
};

vector<Token> tokens;
vector<vector<Token>> lineTokens;
int current_pos = 0;
map<string, string> symbolTable;   // Store: Variable name -> Type [cite: 63-65]
string current_expr_type = "int";  // Tracks expression type for Type Checking

// ==========================================
// ERROR HANDLING HELPER
// ==========================================
void error(const string& msg) {
    cout << "❌ " << msg << "\n";
    exit(1);
}

// ==========================================
// 2. LEXICAL ANALYZER [cite: 51-57]
// ==========================================
bool isKeyword(const string& word) {
    vector<string> keywords = {"int", "float", "if", "else", "while", "print"}; // [cite: 13]
    return find(keywords.begin(), keywords.end(), word) != keywords.end();
}

void tokenize(const string& code) {
    tokens.clear();
    int i = 0, n = code.length();

    while (i < n) {
        if (isspace(code[i])) { i++; continue; }

        // Keywords & Identifiers [cite: 53-54]
        if (isalpha(code[i])) {
            string word = "";
            while (i < n && isalnum(code[i])) { word += code[i]; i++; }
            if (isKeyword(word)) tokens.push_back({"keyword", word});
            else tokens.push_back({"id", word});
            continue;
        }

        // Numbers (Integers & Floats) [cite: 55]
        if (isdigit(code[i])) {
            string num = "";
            while (i < n && (isdigit(code[i]) || code[i] == '.')) { num += code[i]; i++; }
            tokens.push_back({"num", num});
            continue;
        }

        // 2-Character Relational Operators (<=, >=, ==, !=) [cite: 40]
        if (i + 1 < n) {
            string twoCharOp = code.substr(i, 2);
            if (twoCharOp == "<=" || twoCharOp == ">=" || twoCharOp == "==" || twoCharOp == "!=") {
                tokens.push_back({"operator", twoCharOp});
                i += 2;
                continue;
            }
        }

        // Single-Character Operators & Delimiters [cite: 15-17]
        string opsAndDelims = "+-*/=<>[]{};()";
        if (opsAndDelims.find(code[i]) != string::npos) {
            string type = (string("[]{};()").find(code[i]) != string::npos) ? "delimiter" : "operator";
            tokens.push_back({type, string(1, code[i])});
            i++;
            continue;
        }
        
        i++; // Skip unknown characters
    }
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

// Program → StmtList [cite: 21]
void Program() {
    StmtList();
    if (peek().type == "EOF") {
        cout << "✅ Compilation Successful: No Syntax or Semantic errors!\n";
    } else {
        error("Syntax Error: Unrecognized code at the end.");
    }
}

// StmtList → Stmt StmtList | ε [cite: 23]
void StmtList() {
    if (peek().type != "EOF" && peek().value != "}") {
        Stmt();
        StmtList();
    }
}

// Stmt → DeclStmt | AssignStmt | IfStmt | WhileStmt | PrintStmt [cite: 25]
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

// DeclStmt → Type id ; [cite: 27]
void DeclStmt() {
    string varType = consume().value; // consume 'int' or 'float'
    
    if (peek().type != "id") error("Syntax Error: Expected identifier after type.");
    string varName = consume().value;
    
    // Semantic Check: Duplicate declarations [cite: 69]
    if (symbolTable.count(varName)) {
        error("Semantic Error: Variable '" + varName + "' already declared");
    }
    symbolTable[varName] = varType; 
    
    match(";");
}

// AssignStmt → id = Expr ; [cite: 30]
void AssignStmt() {
    string varName = consume().value; 
    
    // Semantic Check: Undeclared variables [cite: 68]
    if (!symbolTable.count(varName)) {
        error("Semantic Error: Variable '" + varName + "' not declared");
    }
    string varType = symbolTable[varName];

    match("=");
    
    current_expr_type = "int"; // Reset before evaluating expression
    Expr();
    
    // Semantic Check: Type mismatch [cite: 70]
    if (varType == "int" && current_expr_type == "float") {
        error("Type Error: Cannot assign float to int");
    }

    match(";");
}

// IfStmt → if ( Cond ) { StmtList } ElsePart [cite: 32]
void IfStmt() {
    match("if"); match("("); Cond(); match(")");
    match("{"); StmtList(); match("}");
    ElsePart();
}

// ElsePart → else { StmtList } | ε [cite: 34]
void ElsePart() {
    if (peek().value == "else") {
        consume();
        match("{"); StmtList(); match("}");
    }
}

// WhileStmt → while ( Cond ) { StmtList } [cite: 35]
void WhileStmt() {
    match("while"); match("("); Cond(); match(")");
    match("{"); StmtList(); match("}");
}

// PrintStmt → print ( Expr ) ; [cite: 36]
void PrintStmt() {
    match("print"); match("("); Expr(); match(")"); match(";");
}

// Cond → Expr RelOp Expr [cite: 38]
void Cond() {
    current_expr_type = "int"; Expr(); 
    RelOp(); 
    current_expr_type = "int"; Expr();
}

// RelOp → < | > | <= | >= | == | != [cite: 40]
void RelOp() {
    string val = peek().value;
    if (val == "<" || val == ">" || val == "<=" || val == ">=" || val == "==" || val == "!=") {
        consume();
    } else {
        error("Syntax Error: Expected Relational Operator, found '" + val + "'");
    }
}

// Expr → Term Expr' [cite: 42]
void Expr() { Term(); ExprPrime(); }

// Expr' → + Term Expr' | - Term Expr' | ε [cite: 44]
void ExprPrime() {
    if (peek().value == "+" || peek().value == "-") {
        consume(); Term(); ExprPrime();
    }
}

// Term → Factor Term' [cite: 46]
void Term() { Factor(); TermPrime(); }

// Term' → * Factor Term' | / Factor Term' | ε [cite: 48]
void TermPrime() {
    if (peek().value == "*" || peek().value == "/") {
        consume(); Factor(); TermPrime();
    }
}

// Factor → id | number | ( Expr ) [cite: 49]
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

// টোকেনগুলো স্ক্রিনে প্রিন্ট করার জন্য
void printTokens() {
    cout << "Expected Token Output:\n";
    for (const auto& t : tokens) {
        if (t.type == "id") cout << "[id:" << t.value << "] ";
        else if (t.type == "num") cout << "[num:" << t.value << "] ";
        else cout << "[" << t.value << "] "; // Keyword, Operator বা Delimiter হলে
        
        if (t.value == ";" or t.value == "{" or t.value == "}") cout << endl;
    }
    cout << "\n\n";
}
// ==========================================
// 5. TEST RUNNER (Main Function)
// ==========================================
void runCompiler(const string& testName, const string& code) {
    cout << "--------------------------------------\n";
    cout << "[" << testName << "]\nInput Code:\n" << code << "\n\nOutput:\n";
    
    symbolTable.clear();
    current_expr_type = "int";
    
    tokenize(code);
    printTokens();

    current_pos = 0;
    
    if (!tokens.empty()) Program();
    cout << "\n";
}

int main() {
    // PDF Manual Test Cases 
    runCompiler("Case 1: Undeclared Variable", "int a;\n a = 10;\n");
    runCompiler("Case 2: Duplicate Declaration", "int a; \nint b; \nb = a;");
    runCompiler("Case 3: Syntax Error (Missing Semicolon)", "int a\n; a = 5;\n"); 
    runCompiler("Case 4: Type Mismatch", "int a; \na = 3;");
    
    // New Cases implemented from Grammar!
    runCompiler("Case 5: If-Else", "int a; \na = 10; \nif (a > 5) {\n \tprint(a); \n} else { \n\tprint(0); \n}");
    runCompiler("Case 6: While Loop", "int a; \na = 3; \nwhile (a > 0) {\n\tprint(a); \n\ta = a - 1; \n}");

    return 0;
}