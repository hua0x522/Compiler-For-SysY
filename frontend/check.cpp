#include "check.h"

namespace CHECK
{
Table table;
string funcType = "";
vector<int> funcArgs;
vector<int> realArgs;
bool noConst = false; 
bool needReturn = false;
int loops = 0;
int blocks = 0;
int expDim = 0;
vector<Err> errs;

void cCompUnit(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Decl>") cDecl(*it);
        else if ((*it)->token.val == "<FuncDef>") cFuncDef(*it);
        else if ((*it)->token.val == "<MainFuncDef>") cMainFuncDef(*it);
    }
}

void cDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstDecl>") cConstDecl(*it);
        else if ((*it)->token.val == "<VarDecl>") cVarDecl(*it);
    }
}

void cConstDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstDef>") cConstDef(*it);
    }
}

void cConstDef(Node* node) {
    Var var("", "const int", 0);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") {
            var.id = (*it)->token.val;
            if (table.has(var.id)) err_log("b", (*it)->token.line);
        }
        else if ((*it)->token.val == "[") var.dim += 1;
        else if ((*it)->token.val == "=") table.add(var);
        else if ((*it)->token.val == "<ConstExp>") cConstExp(*it);
        else if ((*it)->token.val == "<ConstInitVal>") cConstInitVal(*it);
    }
}

void cConstInitVal(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstExp>") cConstExp(*it);
        else if ((*it)->token.val == "<ConstInitVal>") cConstInitVal(*it);
    }
}

void cVarDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<VarDef>") cVarDef(*it);
    }
}

void cVarDef(Node* node) {
    Var var("", "int", 0);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") {
            var.id = (*it)->token.val;
            if (table.has(var.id)) err_log("b", (*it)->token.line);
        }
        else if ((*it)->token.val == "[") var.dim += 1;
        else if ((*it)->token.val == "=") {
            table.add(var); 
            var.dim = -1;
        }
        else if ((*it)->token.val == "<ConstExp>") cConstExp(*it);
        else if ((*it)->token.val == "<InitVal>") cInitVal(*it);
    }
    if (var.dim != -1) table.add(var);
}

void cInitVal(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") cExp(*it);
        else if ((*it)->token.val == "<InitVal>") cInitVal(*it);
    }
}

void cFuncDef(Node* node) {
    Func func;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<FuncType>") {
            cFuncType(*it);
            func.tp = funcType;
        }
        else if ((*it)->token.tp == "IDENFR") {
            func.id = (*it)->token.val;
            if (table.has(func.id)) err_log("b", (*it)->token.line);
            table.inScope();
        }
        else if ((*it)->token.val == "<FuncFParams>") {
            cFuncFParams(*it);
            func.args = funcArgs;
        }
        else if ((*it)->token.val == "<Block>") {
            table.add(func);
            cBlock(*it);
            table.outScope();
        }
    } 
}

void cMainFuncDef(Node* node) {
    Func func; func.id = "main"; func.tp = "int";
    funcType = "int";
    table.inScope();
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Block>") {
            cBlock(*it);
            table.outScope();
        }
    }
}

void cFuncType(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "void") funcType = "void";
        else if ((*it)->token.val == "int") funcType = "int";
    }
}

void cFuncFParams(Node* node) {
    funcArgs.clear();
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<FuncFParam>") cFuncFParam(*it); 
    }
}

void cFuncFParam(Node* node) {
    Var var("","int",0);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") {
            var.id = (*it)->token.val;
            if (table.has(var.id)) err_log("b", (*it)->token.line);
        }
        else if ((*it)->token.val == "[") var.dim += 1;
        else if ((*it)->token.val == "<ConstExp>") cConstExp(*it);
    }
    table.add(var);
    funcArgs.push_back(var.dim);
}

void cBlock(Node* node) {
    if (blocks) table.inScope();
    blocks++;
    if (blocks == 1) needReturn = false;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if (funcType=="int" && blocks==1 && it==node->sons.end()-2) needReturn = true;
        if ((*it)->token.val == "<BlockItem>") {
            cBlockItem(*it);
        }
        else if ((*it)->token.val == "}") {
            if (blocks == 1 && needReturn) err_log("g", (*it)->token.line);
        }
    }
    blocks--;
    if (blocks) table.outScope();  
}

void cBlockItem(Node* node) {
    if (node->sons[0]->token.val == "<Decl>") {
        cDecl(node->sons[0]);
    }
    else if (node->sons[0]->token.val == "<Stmt>") {
        if (blocks == 1 && node->sons[0]->sons[0]->token.val == "return") needReturn = false;
        cStmt(node->sons[0]);
    }
}

int parseStr(string s) {
    int cnt = 0;
    s.erase(s.begin());
    s.erase(s.end()-1);
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '%') {
            if (i == s.size()-1 || s[i+1] != 'd') return -1;
            else cnt++;
        }
        else if (s[i] == '\\') {
            if (i == s.size()-1 || s[i+1] != 'n') return -1;
        }
        else if ((s[i] < 40 || s[i] > 126) && s[i] != 32 && s[i] != 33) {
            return -1;
        }
    }
    return cnt;
}

void cStmt(Node* node) {
    if (node->sons[0]->token.val == "while") {
        cCond(node->sons[2]);
        loops++;
        cStmt(node->sons[4]);
        loops--; 
    }
    else if (node->sons[0]->token.val == "return") {
        if (node->sons[1]->token.val == "<Exp>") {
            if (funcType == "void") err_log("f", node->sons[0]->token.line);
            cExp(node->sons[1]);
        }
    }
    else if (node->sons[0]->token.val == "printf") {
        int left, right = 0;
        for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
            if ((*it)->token.tp == "STRCON") {
                left = parseStr((*it)->token.val);
                if (left == -1) err_log("a", (*it)->token.line);
            }
            else if ((*it)->token.val == "<Exp>") {
                cExp(*it);
                right++;
            }
        }
        if (left >= 0 && left != right) err_log("l", node->sons[0]->token.line);
    }
    else {
        for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
            if ((*it)->token.val == "<LVal>") {
                noConst = true;
                cLVal(*it);
                noConst = false; 
            } 
            else if ((*it)->token.val == "break" || (*it)->token.val == "continue") {
                if (!loops) err_log("m", (*it)->token.line);
            }
            else if ((*it)->token.val == "<Exp>") cExp(*it);
            else if ((*it)->token.val == "<Block>") cBlock(*it);
            else if ((*it)->token.val == "<Stmt>") cStmt(*it);
            else if ((*it)->token.val == "<Cond>") cCond(*it); 
        }
    }
}

void cExp(Node* node) {
    cAddExp(node->sons[0]);
}

void cCond(Node* node) {
    cLOrExp(node->sons[0]);
}

void cLVal(Node* node) {
    string id = node->sons[0]->token.val;
    Var var = table.findVar(id);
    if (var.dim == -1) err_log("c", node->sons[0]->token.line);
    if (var.tp == "const int" && noConst) err_log("h", node->sons[0]->token.line);
    expDim = var.dim;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") {
            int temp = expDim;
            cExp(*it);
            expDim = temp;
        }
        if ((*it)->token.val == "[") expDim--;
    }
}

void cPrimaryExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") cExp(*it);
        else if ((*it)->token.val == "<LVal>") cLVal(*it);
        else if ((*it)->token.val == "<Number>") expDim = 0;
    }
}

int checkPara(vector<int> fArgs) {
    int pos;
    for (pos = realArgs.size()-1; pos >= 0; pos--) {
        if (realArgs[pos] == -7) break;
    }
    vector<int> rArgs;
    for (int i = pos+1; i < realArgs.size(); i++) {
        rArgs.push_back(realArgs[i]);
    }
    while (*(realArgs.end()-1) != -7) realArgs.pop_back();
    realArgs.pop_back();
    if (rArgs.size() != fArgs.size()) return 1;
    for (int i = 0; i < rArgs.size(); i++) {
        if (fArgs[i] != rArgs[i]) return 2;
    }
    return 0;
}

void cUnaryExp(Node* node) {
    if (node->sons[0]->token.val == "<PrimaryExp>") cPrimaryExp(node->sons[0]);
    else if (node->sons[1]->token.val == "<UnaryExp>") cUnaryExp(node->sons[1]);
    else if (node->sons[0]->token.tp == "IDENFR") {
        Func func = table.findFunc(node->sons[0]->token.val);
        if (func.id == "") {
            err_log("c", node->sons[0]->token.line);
        }
        else {
            cFuncRParams(node->sons[2]);
            int state = checkPara(func.args);
            if (state == 1) err_log("d", node->sons[0]->token.line);
            else if (state == 2) err_log("e", node->sons[0]->token.line);
        }
        if (func.tp == "int") expDim = 0;
        else expDim = -1;
    }
}

void cFuncRParams(Node* node) {
    realArgs.push_back(-7);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") {
            expDim = 0;
            cExp(*it); 
            realArgs.push_back(expDim);
        }
    }
}

void cMulExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<UnaryExp>") cUnaryExp(*it);
        else if ((*it)->token.val == "<MulExp>") cMulExp(*it); 
    }
}

void cAddExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<AddExp>") cAddExp(*it);
        else if ((*it)->token.val == "<MulExp>") cMulExp(*it); 
    }
}

void cRelExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<AddExp>") cAddExp(*it);
        else if ((*it)->token.val == "<RelExp>") cRelExp(*it); 
    }
}

void cEqExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<EqExp>") cEqExp(*it);
        else if ((*it)->token.val == "<RelExp>") cRelExp(*it); 
    }
}

void cLAndExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<EqExp>") cEqExp(*it);
        else if ((*it)->token.val == "<LAndExp>") cLAndExp(*it); 
    }
}

void cLOrExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<LOrExp>") cLOrExp(*it);
        else if ((*it)->token.val == "<LAndExp>") cLAndExp(*it); 
    }
}

void cConstExp(Node* node) {
    cAddExp(node->sons[0]);
}

bool cmp(Err e1, Err e2) {
    return e1.line < e2.line;
}

void err_log(string err, int line) {
    Err e(err, line);
    errs.push_back(e);
}

void check(Node* node) {
    cCompUnit(node);
    sort(errs.begin(), errs.end(), cmp);
    // FILE* err = fopen("error.txt", "w");
    // for (int i = 0; i < errs.size(); i++) {
    //     fprintf(err, "%d %s\n", errs[i].line, errs[i].err.c_str());
    // }
    // fclose(err);
}
}