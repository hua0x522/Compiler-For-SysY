#include "analyser.h"
#include "token.h"

void readStr(Node* node, vector<Token>& tokens, string target) {
    Node* son; 
    if (target == tokens[0].val) {
        son = new Node(tokens[0], node);
        tokens.erase(tokens.begin());  
    }
    else son = new Node("<error>", node);
}

void readStr(Node* node, vector<Token>& tokens) {
    Node* son = new Node(tokens[0], node);
    tokens.erase(tokens.begin());  
}

void CompUnit(Node* node, vector<Token>& tokens) {
    Node* son;
    while (tokens[0].val == "const" || (tokens[0].val == "int" && tokens[2].val != "(")) {
        son = new Node("<Decl>", node);
        Decl(son, tokens);
    }
    while (tokens[0].val == "void" || (tokens[0].val == "int" && tokens[1].val != "main")) {
        son = new Node("<FuncDef>", node);
        FuncDef(son, tokens);
    }
    son = new Node("<MainFuncDef>", node);
    MainFuncDef(son, tokens);
}

void Decl(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "const") {
        son = new Node("<ConstDecl>", node);
        ConstDecl(son, tokens);
    }
    else if (tokens[0].val == "int") {
        son = new Node("<VarDecl>", node);
        VarDecl(son, tokens);
    }
}

void ConstDecl(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens, "const");
    readStr(node, tokens, "int");
    son = new Node("<ConstDef>", node);
    ConstDef(son, tokens);
    while(tokens[0].val == ",") {
        readStr(node, tokens);
        son = new Node("<ConstDef>", node);
        ConstDef(son, tokens);
    }
    readStr(node, tokens, ";");
}

void ConstDef(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens);  //IDENFR
    while (tokens[0].val == "[") {
        readStr(node, tokens);
        son = new Node("<ConstExp>", node);
        ConstExp(son, tokens);
        readStr(node, tokens, "]");
    }
    readStr(node, tokens, "=");
    son = new Node("<ConstInitVal>", node);
    ConstInitVal(son, tokens);
}

void ConstInitVal(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "{") {  
        readStr(node, tokens);
        if (tokens[0].val == "}") {
            readStr(node, tokens);
        }
        else {
            son = new Node("<ConstInitVal>", node);
            ConstInitVal(son, tokens);
            while (tokens[0].val == ",") {
                readStr(node, tokens);
                son = new Node("<ConstInitVal>", node);
                ConstInitVal(son, tokens);
            }
            readStr(node, tokens, "}");
        }  
    }
    else {
        son = new Node("<ConstExp>", node);
        ConstExp(son, tokens);
    }
}

void ConstExp(Node* node, vector<Token>& tokens) {
    Node* son = new Node("<AddExp>", node);
    AddExp(son, tokens);
}

void AddExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<MulExp>", node);
    MulExp(son, tokens);
    while (tokens[0].val == "-" || tokens[0].val == "+") {
        readStr(node, tokens);
        son = new Node("<MulExp>", node);
        MulExp(son, tokens);
    }
}

void MulExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<UnaryExp>", node);
    UnaryExp(son, tokens);
    while (tokens[0].val == "*" || tokens[0].val == "/" || tokens[0].val == "%") {
        readStr(node, tokens);
        son = new Node("<UnaryExp>", node);
        UnaryExp(son, tokens);
    }
}

void UnaryExp(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "+" || tokens[0].val == "-" || tokens[0].val == "!") {
        son = new Node("<UnaryOp>", node);
        UnaryOp(son, tokens); 
        son = new Node("<UnaryExp>", node);
        UnaryExp(son, tokens);
    }
    else if (tokens[0].val == "(" || tokens[0].tp == "INTCON") {
        son = new Node("<PrimaryExp>", node);
        PrimaryExp(son, tokens);
    }
    else if (tokens[0].tp == "IDENFR") {
        if (tokens[1].val == "(") { 
            readStr(node, tokens);  //IDENFR
            readStr(node, tokens); 
            if (tokens[0].val == ")") {
                readStr(node, tokens);
            }
            else {
                son = new Node("<FuncRParams>", node);
                FuncRParams(son, tokens);
                readStr(node, tokens, ")");
            }
        }
        else {
            son = new Node("<PrimaryExp>", node);
            PrimaryExp(son, tokens);
        }
    }
}

void UnaryOp(Node* node, vector<Token>& tokens) {
    if (tokens[0].val == "+" || tokens[0].val == "-" || tokens[0].val == "!") {
        readStr(node, tokens);
    }
}

void PrimaryExp(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "(") {
        readStr(node, tokens); 
        son = new Node("<Exp>", node);
        Exp(son, tokens);
        readStr(node, tokens, ")");
    }
    else if (tokens[0].tp == "IDENFR") {
        son = new Node("<LVal>", node);
        LVal(son, tokens);
    }
    else if (tokens[0].tp == "INTCON") {
        son = new Node("<Number>", node);
        Number(son, tokens);
    }
}

void LVal(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens);  //IDENFR
    while (tokens[0].val == "[") {
        readStr(node, tokens); 
        son = new Node("<Exp>", node);
        Exp(son, tokens);
        readStr(node, tokens, "]");
    }
}

void Number(Node* node, vector<Token>& tokens) {
    if (tokens[0].tp == "INTCON") {
        readStr(node, tokens);
    }
}

void Exp(Node* node, vector<Token>& tokens) {
    Node* son = new Node("<AddExp>", node);
    AddExp(son, tokens);
}

void VarDecl(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens, "int");
    son = new Node("<VarDef>", node);
    VarDef(son, tokens);
    while (tokens[0].val == ",") {
        readStr(node, tokens);
        son = new Node("<VarDef>", node);
        VarDef(son, tokens);
    }
    readStr(node, tokens, ";");
}

void VarDef(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens);  //IDENFR
    while (tokens[0].val == "[") {
        readStr(node, tokens);
        son = new Node("<ConstExp>", node);
        ConstExp(son, tokens);
        readStr(node, tokens, "]");
    }
    if (tokens[0].val == "=") {
        readStr(node, tokens);
        son = new Node("<InitVal>", node);
        InitVal(son, tokens);
    }
}

void InitVal(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "{") {
        readStr(node, tokens);
        if (tokens[0].val == "}") {
            readStr(node, tokens);
        }
        else {
            son = new Node("<InitVal>", node);
            InitVal(son, tokens);
            while (tokens[0].val == ",") {
                readStr(node, tokens);
                son = new Node("<InitVal>", node);
                InitVal(son, tokens);
            }
            readStr(node, tokens, "}");
        } 
    }
    else {
        son = new Node("<Exp>", node);
        Exp(son, tokens);
    }
}

void FuncDef(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<FuncType>", node);
    FuncType(son, tokens);
    readStr(node, tokens);  //IDENFR
    readStr(node, tokens, "(");
    if (tokens[0].val == ")") {
        readStr(node, tokens);
    }
    else {
        son = new Node("<FuncFParams>", node);
        FuncFParams(son, tokens);
        readStr(node, tokens, ")");
    }
    son = new Node("<Block>", node);
    Block(son, tokens);
}

void FuncType(Node* node, vector<Token>& tokens) {
    if (tokens[0].val == "void" || tokens[0].val == "int") {
        readStr(node, tokens);
    }
}

void FuncFParams(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<FuncFParam>", node);
    FuncFParam(son, tokens);
    while (tokens[0].val == ",") {
        readStr(node, tokens);
        son = new Node("<FuncFParam>", node);
        FuncFParam(son, tokens);
    }
}

void FuncFParam(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens, "int");
    readStr(node, tokens);  //IDENFR
    if (tokens[0].val == "[") {
        readStr(node, tokens);
        readStr(node, tokens, "]");
        while (tokens[0].val == "[") {
            readStr(node, tokens);
            son = new Node("<ConstExp>", node);
            ConstExp(son, tokens);
            readStr(node, tokens, "]");
        }
    }
}

void Block(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens, "{");
    while (tokens[0].val != "}") {
        son = new Node("<BlockItem>", node);
        BlockItem(son, tokens);
    }
    readStr(node, tokens, "}"); 
}

void BlockItem(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "const" || tokens[0].val == "int") {
        son = new Node("<Decl>", node);
        Decl(son, tokens);
    }
    else {
        son = new Node("<Stmt>", node);
        Stmt(son, tokens);
    }
}

Node* findLVal(Node* node) {
    while (node->token.val != "<LVal>") {
        node = node->sons[0];
    }
    return node;
}

void Stmt(Node* node, vector<Token>& tokens) {
    Node* son;
    if (tokens[0].val == "{") {
        son = new Node("<Block>", node);
        Block(son, tokens);
    }
    else if (tokens[0].val == "if") {
        readStr(node, tokens); 
        readStr(node, tokens, "(");
        son = new Node("<Cond>", node);
        Cond(son, tokens);
        readStr(node, tokens, ")");
        son = new Node("<Stmt>", node);
        Stmt(son, tokens);
        if (tokens[0].val == "else") {
            readStr(node, tokens);
            son = new Node("<Stmt>", node);
            Stmt(son, tokens);
        }
    }
    else if (tokens[0].val == "while") {
        readStr(node, tokens);
        readStr(node, tokens, "(");
        son = new Node("<Cond>", node);
        Cond(son, tokens);
        readStr(node, tokens, ")");
        son = new Node("<Stmt>", node);
        Stmt(son, tokens);
    }
    else if (tokens[0].val == "break") {
        readStr(node, tokens);
        readStr(node, tokens, ";");
    }
    else if (tokens[0].val == "continue") {
        readStr(node, tokens);
        readStr(node, tokens, ";");
    }
    else if (tokens[0].val == "return") {
        readStr(node, tokens);
        if (tokens[0].val != ";") {
            son = new Node("<Exp>", node);
            Exp(son, tokens);
        }
        readStr(node, tokens, ";");
    }
    else if (tokens[0].val == "printf") {
        readStr(node, tokens);
        readStr(node, tokens, "(");
        readStr(node, tokens);  //STRCON
        while (tokens[0].val == ",") {
            readStr(node, tokens);
            son = new Node("<Exp>", node);
            Exp(son, tokens);
        }
        readStr(node, tokens, ")");
        readStr(node, tokens, ";");
    }
    else if (tokens[0].val == ";") {
        readStr(node, tokens);
    }
    else {
        son = new Node("<Exp>", node);
        Exp(son, tokens);
        if (tokens[0].val == "=") {
            node->add(findLVal(son));
            node->del(son);
            readStr(node, tokens);
            if (tokens[0].val == "getint") {
                readStr(node, tokens);
                readStr(node, tokens, "(");
                readStr(node, tokens, ")");
                readStr(node, tokens, ";");
            }
            else {
                son = new Node("<Exp>", node);
                Exp(son, tokens);
                readStr(node, tokens, ";");
            }
        }
        else readStr(node, tokens, ";");
    }
}

void Cond(Node* node, vector<Token>& tokens) {
    Node* son = new Node("<LOrExp>", node);
    LOrExp(son, tokens); 
}

void LOrExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<LAndExp>", node);
    LAndExp(son, tokens);
    while (tokens[0].val == "||") {
        readStr(node, tokens);
        son = new Node("<LAndExp>", node);
        LAndExp(son, tokens);
    }
}

void LAndExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<EqExp>", node);
    EqExp(son, tokens);
    while (tokens[0].val == "&&") {
        readStr(node, tokens);
        son = new Node("<EqExp>", node);
        EqExp(son, tokens);
    }
}

void EqExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<RelExp>", node);
    RelExp(son, tokens);
    while (tokens[0].val == "==" || tokens[0].val == "!=") {
        readStr(node, tokens);
        son = new Node("<RelExp>", node);
        RelExp(son, tokens);
    }
}


void RelExp(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<AddExp>", node);
    AddExp(son, tokens);
    while (tokens[0].val == "<" || tokens[0].val == "<=" || tokens[0].val == ">" || tokens[0].val == ">=") {
        readStr(node, tokens); 
        son = new Node("<AddExp>", node);
        AddExp(son, tokens);
    } 
} 

void FuncRParams(Node* node, vector<Token>& tokens) {
    Node* son;
    son = new Node("<Exp>", node);
    Exp(son, tokens);
    while (tokens[0].val == ",") {
        readStr(node, tokens);
        son = new Node("<Exp>", node);
        Exp(son, tokens);
    }
}

void MainFuncDef(Node* node, vector<Token>& tokens) {
    Node* son;
    readStr(node, tokens, "int");
    readStr(node, tokens, "main");
    readStr(node, tokens, "(");
    readStr(node, tokens, ")");
    son = new Node("<Block>", node);
    Block(son, tokens);
}

void print_tree(Node* node, FILE* fp) {
    if (!node->sons.size()) {
        fprintf(fp, "%s %s\n", node->token.tp.c_str(), node->token.val.c_str());
        return;
    }
    for (int i = 0; i < node->sons.size(); i++) {
        print_tree(node->sons[i], fp);
        if ((node->token.val == "<MulExp>" || node->token.val == "<AddExp>" || node->token.val == "<RelExp>"
        || node->token.val == "<EqExp>" || node->token.val == "<LAndExp>" || node->token.val == "<LOrExp>")
        && i % 2 == 0 && i < node->sons.size()-1) {
            fprintf(fp, "%s\n", node->token.val.c_str());
        }
    }
    if (node->token.val != "<BlockItem>" && node->token.val != "<Decl>" && node->token.val != "<BType>") {
        fprintf(fp, "%s\n", node->token.val.c_str());
    }
}

void LL() {
    vector<Token> tokens = lexer(); 
    vector<Token> q(tokens);
    Node* root = new Node("<CompUnit>", NULL);  
    CompUnit(root, q);
    FILE* fp = fopen("output.txt", "w");
    print_tree(root, fp);
    fclose(fp); 
}