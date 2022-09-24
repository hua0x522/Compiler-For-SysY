#include "analyser.h"

map<string, string> typeMap;

string getType(string str) {
    if (typeMap.count(str)) {
        return typeMap[str];
    }
    else {
        if (str[0] == '\"') return "STRCON";
        else if (isDigit(str[0])) return "INTCON";
        else return "IDENFR";
    }
}

void initMap() {
    typeMap["main"] = "MAINTK";
    typeMap["const"] = "CONSTTK";
    typeMap["int"] = "INTTK";
    typeMap["break"] = "BREAKTK";
    typeMap["continue"] = "CONTINUETK";
    typeMap["if"] = "IFTK";    
    typeMap["else"] = "ELSETK";
    typeMap["!"] = "NOT";
    typeMap["&&"] = "AND";
    typeMap["||"] = "OR";
    typeMap["while"] = "WHILETK";
    typeMap["getint"] = "GETINTTK";
    typeMap["printf"] = "PRINTFTK";
    typeMap["return"] = "RETURNTK";
    typeMap["+"] = "PLUS";
    typeMap["-"] = "MINU";
    typeMap["void"] = "VOIDTK";
    typeMap["*"] = "MULT";
    typeMap["/"] = "DIV";   
    typeMap["%"] = "MOD"; 
    typeMap["<"] = "LSS";
    typeMap["<="] = "LEQ";
    typeMap[">"] = "GRE";
    typeMap[">="] = "GEQ";
    typeMap["=="] = "EQL";
    typeMap["!="] = "NEQ";
    typeMap["="] = "ASSIGN";
    typeMap[";"] = "SEMICN";
    typeMap[","] = "COMMA";
    typeMap["("] = "LPARENT";
    typeMap[")"] = "RPARENT";
    typeMap["["] = "LBRACK";
    typeMap["]"] = "RBRACK";
    typeMap["{"] = "LBRACE";
    typeMap["}"] = "RBRACE"; 
}

bool CompUnit(Node* node, vector<string>& tokens) {
    Node* son;
    while (tokens[0] == "const" || (tokens[0] == "int" && tokens[2] != "(")) {
        son = new Node("<Decl>", node);
        if (!Decl(son, tokens)) return false;
    }
    while (tokens[0] == "void" || (tokens[0] == "int" && tokens[1] != "main")) {
        son = new Node("<FuncDef>", node);
        if (!FuncDef(son, tokens)) return false;
    }
    son = new Node("<MainFuncDef>", node);
    if (!MainFuncDef(son, tokens)) return false;
    return true;
}

bool Decl(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "const") {
        son = new Node("<ConstDecl>", node);
        ConstDecl(son, tokens);
    }
    else if (tokens[0] == "int") {
        son = new Node("<VarDecl>", node);
        VarDecl(son, tokens);
    }
    else return false;
    return true;
}

bool ConstDecl(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] != "const") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] != "int") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    son = new Node("<ConstDef>", node);
    if(!ConstDef(son, tokens)) return false;
    while(tokens[0] == ",") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<ConstDef>", node);
        if (!ConstDef(son, tokens)) return false;
    }
    if (tokens[0] != ";") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    return true;
}

bool ConstDef(Node* node, vector<string>& tokens) {
    Node* son;
    if (getType(tokens[0]) != "IDENFR") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    while (tokens[0] == "[") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<ConstExp>", node);
        if (!ConstExp(son, tokens)) return false;
        if (tokens[0] != "]") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    if (tokens[0] != "=") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    son = new Node("<ConstInitVal>", node);
    if (!ConstInitVal(son, tokens)) return false;
    return true;
}

bool ConstInitVal(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "{") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] == "}") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        }
        else {
            son = new Node("<ConstInitVal>", node);
            if (!ConstInitVal(son, tokens)) return false;
            while (tokens[0] == ",") {
                son = new Node(tokens[0], node);
                tokens.erase(tokens.begin());
                son = new Node("<ConstInitVal>", node);
                if (!ConstInitVal(son, tokens)) return false;
            }
            if (tokens[0] != "}") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        } 
    }
    else {
        son = new Node("<ConstExp>", node);
        if (!ConstExp(son, tokens)) return false;
    }
    return true;
}

bool ConstExp(Node* node, vector<string>& tokens) {
    Node* son = new Node("<AddExp>", node);
    if (!AddExp(son, tokens)) return false;
    return true;
}

bool AddExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<MulExp>", node);
    if (!MulExp(son, tokens)) return false;
    while (tokens[0] == "-" || tokens[0] == "+") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<MulExp>", node);
        if (!MulExp(son, tokens)) return false;
    }
    return true;
}

bool MulExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<UnaryExp>", node);
    if (!UnaryExp(son, tokens)) return false;
    while (tokens[0] == "*" || tokens[0] == "/" || tokens[0] == "%") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<UnaryExp>", node);
        if (!UnaryExp(son, tokens)) return false;
    }
    return true;
}

bool UnaryExp(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "+" || tokens[0] == "-" || tokens[0] == "!") {
        son = new Node("<UnaryOp>", node);
        if (!UnaryOp(son, tokens)) return false;
        son = new Node("<UnaryExp>", node);
        if (!UnaryExp(son, tokens)) return false;
    }
    else if (tokens[0] == "(" || getType(tokens[0]) == "INTCON") {
        son = new Node("<PrimaryExp>", node);
        if (!PrimaryExp(son, tokens)) return false;
    }
    else if (getType(tokens[0]) == "IDENFR") {
        if (tokens[1] == "(") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            if (tokens[0] == ")") {
                son = new Node(tokens[0], node);
                tokens.erase(tokens.begin());
            }
            else {
                son = new Node("<FuncRParams>", node);
                if (!FuncRParams(son, tokens)) return false;
                if (tokens[0] != ")") return false;
                son = new Node(tokens[0], node);
                tokens.erase(tokens.begin());
            }
        }
        else {
            son = new Node("<PrimaryExp>", node);
            if (!PrimaryExp(son, tokens)) return false;
        }
    }
    else return false;
    return true;
}

bool UnaryOp(Node* node, vector<string>& tokens) {
    if (tokens[0] == "+" || tokens[0] == "-" || tokens[0] == "!") {
        Node* son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        return true;
    }
    return false;
}

bool PrimaryExp(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "(") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Exp>", node);
        if (!Exp(son, tokens)) return false;
        if (tokens[0] != ")") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else if (getType(tokens[0]) == "IDENFR") {
        son = new Node("<LVal>", node);
        if (!LVal(son, tokens)) return false;
    }
    else if (getType(tokens[0]) == "INTCON") {
        son = new Node("<Number>", node);
        if (!Number(son, tokens)) return false;
    }
    else return false;
    return true;
}

bool LVal(Node* node, vector<string>& tokens) {
    Node* son;
    if (getType(tokens[0]) != "IDENFR") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin()); 
    while (tokens[0] == "[") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Exp>", node);
        if(!Exp(son, tokens)) return false;
        if (tokens[0] != "]") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    return true;
}

bool Number(Node* node, vector<string>& tokens) {
    if (getType(tokens[0]) == "INTCON") {
        Node* son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        return true;
    }
    else return false;
}

bool Exp(Node* node, vector<string>& tokens) {
    Node* son = new Node("<AddExp>", node);
    if(!AddExp(son, tokens)) return false;
    return true;
}

bool VarDecl(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] != "int") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    son = new Node("<VarDef>", node);
    if (!VarDef(son, tokens)) return false;
    while (tokens[0] == ",") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<VarDef>", node);
        if (!VarDef(son, tokens)) return false;
    }
    if (tokens[0] != ";") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    return true;
}

bool VarDef(Node* node, vector<string>& tokens) {
    Node* son;
    if (getType(tokens[0]) != "IDENFR") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    while (tokens[0] == "[") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<ConstExp>", node);
        if (!ConstExp(son, tokens)) return false;
        if (tokens[0] != "]") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    if (tokens[0] == "=") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<InitVal>", node);
        if (!InitVal(son, tokens)) return false;
    }
    return true;
}

bool InitVal(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "{") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] == "}") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        }
        else {
            son = new Node("<InitVal>", node);
            if (!InitVal(son, tokens)) return false;
            while (tokens[0] == ",") {
                son = new Node(tokens[0], node);
                tokens.erase(tokens.begin());
                son = new Node("<InitVal>", node);
                if (!InitVal(son, tokens)) return false;
            }
            if (tokens[0] != "}") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        } 
    }
    else {
        son = new Node("<Exp>", node);
        if(!Exp(son, tokens)) return false;
    }
    return true;
}

bool FuncDef(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<FuncType>", node);
    if (!FuncType(son, tokens)) return false;
    if (getType(tokens[0]) != "IDENFR") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] != "(") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] == ")") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else {
        son = new Node("<FuncFParams>", node);
        if (!FuncFParams(son, tokens)) return false;
        if (tokens[0] != ")") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    son = new Node("<Block>", node);
    if (!Block(son, tokens)) return false;
    return true;
}

bool FuncType(Node* node, vector<string>& tokens) {
    if (tokens[0] == "void" || tokens[0] == "int") {
        Node* son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        return true;
    }
    else return false;
}

bool FuncFParams(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<FuncFParam>", node);
    if (!FuncFParam(son, tokens)) return false;
    while (tokens[0] == ",") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<FuncFParam>", node);
        if (!FuncFParam(son, tokens)) return false;
    }
    return true;
}

bool FuncFParam(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] != "int") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (getType(tokens[0]) != "IDENFR") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] == "[") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != "]") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        while (tokens[0] == "[") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            son = new Node("<ConstExp>", node);
            if (!ConstExp(son, tokens)) return false;
            if (tokens[0] != "]") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        }
    }
    return true;
}

bool Block(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] != "{") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    while (tokens[0] != "}") {
        son = new Node("<BlockItem>", node);
        if (!BlockItem(son, tokens)) return false;
    }
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    return true;
}

bool BlockItem(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] == "const" || tokens[0] == "int") {
        son = new Node("<Decl>", node);
        if (!Decl(son, tokens)) return false;
    }
    else {
        son = new Node("<Stmt>", node);
        if (!Stmt(son, tokens)) return false;
    }
    return true;
}

bool hasAssign (vector<string>& tokens) {
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "=") return true;
        if (tokens[i] == ";") return false;
    }
    return false;
}

bool Stmt(Node* node, vector<string>& tokens) {
    Node* son;
    if (getType(tokens[0]) == "IDENFR" && hasAssign(tokens)) {
        son = new Node("<LVal>", node);
        if (!LVal(son, tokens)) return false;
        if (tokens[0] != "=") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] == "getint") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            if (tokens[0] != "(") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            if (tokens[0] != ")") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            if (tokens[0] != ";") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        }
        else {
            son = new Node("<Exp>", node);
            if (!Exp(son, tokens)) return false;
            if (tokens[0] != ";") return false;
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
        }
    }
    else if (tokens[0] == "{") {
        son = new Node("<Block>", node);
        if (!Block(son, tokens)) return false;
    }
    else if (tokens[0] == "if") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != "(") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Cond>", node);
        if (!Cond(son, tokens)) return false;
        if (tokens[0] != ")") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Stmt>", node);
        if (!Stmt(son, tokens)) return false;
        if (tokens[0] == "else") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            son = new Node("<Stmt>", node);
            if (!Stmt(son, tokens)) return false;
        }
    }
    else if (tokens[0] == "while") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != "(") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Cond>", node);
        if (!Cond(son, tokens)) return false;
        if (tokens[0] != ")") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Stmt>", node);
        if (!Stmt(son, tokens)) return false;
    }
    else if (tokens[0] == "break") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != ";") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else if (tokens[0] == "continue") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != ";") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else if (tokens[0] == "return") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != ";") {
            son = new Node("<Exp>", node);
            if (!Exp(son, tokens)) return false;
        }
        if (tokens[0] != ";") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else if (tokens[0] == "printf") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != "(") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (getType(tokens[0]) != "STRCON") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        while (tokens[0] == ",") {
            son = new Node(tokens[0], node);
            tokens.erase(tokens.begin());
            son = new Node("<Exp>", node);
            if (!Exp(son, tokens)) return false;
        }
        if (tokens[0] != ")") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        if (tokens[0] != ";") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else if (tokens[0] ==";") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    else {
        son = new Node("<Exp>", node);
        if (!Exp(son, tokens)) return false;
        if (tokens[0] != ";") return false;
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
    }
    return true;
}

bool Cond(Node* node, vector<string>& tokens) {
    Node* son = new Node("<LOrExp>", node);
    if (!LOrExp(son, tokens)) return false;
    return true;
}

bool LOrExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<LAndExp>", node);
    if (!LAndExp(son, tokens)) return false;
    while (tokens[0] == "||") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<LAndExp>", node);
        if (!LAndExp(son, tokens)) return false;
    }
    return true;
}

bool LAndExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<EqExp>", node);
    if (!EqExp(son, tokens)) return false;
    while (tokens[0] == "&&") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<EqExp>", node);
        if (!EqExp(son, tokens)) return false;
    }
    return true;
}

bool EqExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<RelExp>", node);
    if (!RelExp(son, tokens)) return false;
    while (tokens[0] == "==" || tokens[0] == "!=") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<RelExp>", node);
        if (!RelExp(son, tokens)) return false;
    }
    return true;
}


bool RelExp(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<AddExp>", node);
    if (!AddExp(son, tokens)) return false;
    while (tokens[0] == "<" || tokens[0] == "<=" || tokens[0] == ">" || tokens[0] == ">=") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<AddExp>", node);
        if (!AddExp(son, tokens)) return false;
    }
    return true;
}

bool FuncRParams(Node* node, vector<string>& tokens) {
    Node* son;
    son = new Node("<Exp>", node);
    if (!Exp(son, tokens)) return false;
    while (tokens[0] == ",") {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());
        son = new Node("<Exp>", node);
        if (!Exp(son, tokens)) return false;
    }
    return true;
}

bool MainFuncDef(Node* node, vector<string>& tokens) {
    Node* son;
    if (tokens[0] != "int") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] != "main") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] != "(") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    if (tokens[0] != ")") return false;
    son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());
    son = new Node("<Block>", node);
    if (!Block(son, tokens)) return false;
    return true; 
}

void print_tree(Node* node, FILE* fp) {
    if (!node->sons.size()) {
        fprintf(fp, "%s %s\n", getType(node->type).c_str(), node->type.c_str());
        return;
    }
    for (int i = 0; i < node->sons.size(); i++) {
        print_tree(node->sons[i], fp);
        if ((node->type == "<MulExp>" || node->type == "<AddExp>" || node->type == "<RelExp>"
        || node->type == "<EqExp>" || node->type == "<LAndExp>" || node->type == "<LOrExp>")
        && i % 2 == 0 && i < node->sons.size()-1) {
            fprintf(fp, "%s\n", node->type.c_str());
        }
    }
    if (node->type != "<BlockItem>" && node->type != "<Decl>" && node->type != "<BType>") {
        fprintf(fp, "%s\n", node->type.c_str());
    }
}

void LL() {
    initMap();
    vector<string> tokens = lexer(); 
    vector<string> q(tokens);
    Node* root = new Node("<CompUnit>", NULL);    
    bool valid = CompUnit(root, q);
    // printf("%d\n", valid);
    FILE* fp = fopen("output.txt", "w");
    print_tree(root, fp);
    fclose(fp); 
}