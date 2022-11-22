#include "generate.h"

namespace GEN 
{
extern Table table;
extern vector<int> cbuf;
extern IR ir;

/*   for clac constant   */
int conAddExp(Node* node) {
    int flag = 1;
    int num = 0;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<MulExp>") num += flag * conMulExp(*it);
        else if ((*it)->token.val == "+") flag = 1;
        else if ((*it)->token.val == "-") flag = -1;
    }
    return num;
}

int conMulExp(Node* node) {
    int flag = 0;
    int num = 0;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<UnaryExp>") {
            if (flag == 0) num = conUnaryExp(*it);
            if (flag == 1) num *= conUnaryExp(*it);
            if (flag == 2) num /= conUnaryExp(*it);
            if (flag == 3) num %= conUnaryExp(*it);  
        }
        else if ((*it)->token.val == "*") flag = 1;
        else if ((*it)->token.val == "/") flag = 2;
        else if ((*it)->token.val == "%") flag = 3;
    }
    return num;
}

int conUnaryExp(Node* node) {
    int flag = 1;
    int num = 0;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<PrimaryExp>") return conPrimaryExp(*it);
        else if ((*it)->token.val == "<UnaryOp>") flag = ((*it)->sons[0]->token.val == "-")? -1 : 1;
        else if ((*it)->token.val == "<UnaryExp>") return flag * conUnaryExp(*it);
    }
    return num;
}

int conPrimaryExp(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") return conExp(*it);
        else if ((*it)->token.val == "<LVal>") return conLVal(*it);
        else if ((*it)->token.val == "<Number>") return conNumber(*it);
    }
    return 0;
}

int conNumber(Node* node) {
    string s = node->sons[0]->token.val;
    int num = 0;
    for (int i = 0; i < s.size(); i++) {
        num = num * 10 + s[i] - '0';
    }
    return num;
}

int conExp(Node* node) {
    return conAddExp(node->sons[0]);
}

int conLVal(Node* node) {
    Var var("", "int", 0);
    vector<int> dims;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") var = table.findVar((*it)->token.val);
        else if ((*it)->token.val == "<Exp>") dims.push_back(conExp(*it));
    }
    int base = 1;
    int pos = 0;
    for (int i = dims.size()-1; i >= 0; i--) {
        pos += base * dims[i];
        base *= var.dims[i];
    }
    return cbuf[var.pos + pos];
}

int gConstExp(Node* node) {
    return conAddExp(node->sons[0]);
}

/*     for IR string     */
void printIR(FILE* fp) {
    fprintf(fp, "declare i32 @getint()\n");
    fprintf(fp, "declare void @putint(i32)\n");
    fprintf(fp, "declare void @putstr(i8*)\n");
    printBlk(ir.global, fp);
    for (int i = 0; i < ir.functions.size(); i++) {
        printFunction(ir.functions[i], fp);
    }
}

void printFunction(Function& func, FILE* fp) {
    if (func.ret.reg == "@getint" || func.ret.reg == "@putint" || func.ret.reg == "@putstr") {
        return;
    }
    fprintf(fp, "%s {\n", func.toString().c_str());
    for (int i = 0; i < func.blks.size(); i++) {
        printBlk(func.blks[i], fp);
    }
    fprintf(fp, "}\n");
}

void printBlk(Blk& blk, FILE* fp) {
    for (int i = 0; i < blk.insts.size(); i++) {
        fprintf(fp, "    %s\n", blk.insts[i].toString().c_str());
    }
}

}