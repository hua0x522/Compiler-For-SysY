#include "generate.h"

namespace GEN 
{
extern Table table;
extern vector<int> cbuf;

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
void printIR(IR& ir, FILE* fp) {
    fprintf(fp, "declare i32 @getint()\n");
    fprintf(fp, "declare void @putint(i32)\n");
    fprintf(fp, "declare void @putstr(i8*)\n");
    printBlk(ir.global, fp);
    for (int i = 0; i < ir.functions.size(); i++) {
        printFunction(ir.functions[i], fp);
    }
}

string ff;

void printFunction(Function& func, FILE* fp) {
    if (func.ret.reg == "@getint" || func.ret.reg == "@putint" || func.ret.reg == "@putstr") {
        return;
    }
    ff = func.ret.reg;
    fprintf(fp, "%s {\n", func.toString().c_str());
    for (int i = 0; i < func.blks.size(); i++) {
        printBlk(func.blks[i], fp);
    }
    fprintf(fp, "}\n");
}

void printBlk(Blk& blk, FILE* fp) {
    for (int i = 0; i < blk.insts.size(); i++) {
        if (blk.insts[i].name == "label")
            fprintf(fp, "%s\n", blk.insts[i].toString().c_str());
        else
            fprintf(fp, "  %s\n", blk.insts[i].toString().c_str());
    }
}

void addMove(Blk& precBlk, Blk& succBlk) {
    vector<Value> to;
    vector<Value> from;
    for (auto it = succBlk.insts.begin(); it != succBlk.insts.end(); it++) {
        if (it->name == "phi") {
            to.push_back(it->ops[0]);
            for (auto op = it->ops.begin(); op != it->ops.end(); op++) {
                if (op->toString() == precBlk.name) {
                    from.push_back(*(op-1));
                }
            }
        }
    }
    auto it = to.begin();
    while (it != to.end()) {
        bool found = false;
        for (int i = 0; i < from.size(); i++) {
            if (from[i].toString() == it->toString()) {
                found = true;
            }
        }
        if (!found) {
            int off = it - to.begin();
            Inst mv("move", 2, *(from.begin() + off), *it);
            to.erase(it);
            from.erase(from.begin() + off);
            precBlk.add(mv);
        }
        else it++;
    }
    vector<Value> temp;
    for (it = from.begin(); it != from.end(); it++) {
        Value v = Value(newReg(), it->ty);
        temp.push_back(v);
        Inst mv("move", 2, *it, v);
        precBlk.add(mv);
    }
    for (int i = 0; i < to.size(); i++) {
        Inst mv("move", 2, temp[i], to[i]);
        precBlk.add(mv);
    }
}

void removePhi(Function& f) {
    for (auto bb = f.blks.begin(); bb != f.blks.end(); bb++) {
        Inst inst = *(bb->insts.end() - 1);
        if (inst.name == "br") {
            bb->insts.erase(bb->insts.end() - 1);
            if (inst.ops.size() == 1) {
                int idx = f.getBlk(inst.ops[0].toString());
                addMove(*bb, *(f.blks.begin() + idx));
                bb->add(inst);
            }
            else {
                int idx1 = f.getBlk(inst.ops[1].toString());
                int idx2 = f.getBlk(inst.ops[2].toString());
                Value label1 = Value(newLabel(),Type());
                Value label2 = Value(newLabel(), Type());
                Inst l1("label", 1, label1);
                Inst l2("label", 1, label2);
                Inst br("br", 3, inst.ops[0], label1, label2);
                Inst br1("br", 1, inst.ops[1]);
                Inst br2("br", 1, inst.ops[2]);
                bb->add(br);
                bb->add(l1);
                addMove(*bb, *(f.blks.begin() + idx1));
                bb->add(br1);
                bb->add(l2);
                addMove(*bb, *(f.blks.begin() + idx2));
                bb->add(br2);
            }
        }
    }

    for (auto bb = f.blks.begin(); bb != f.blks.end(); bb++) {
        auto it = bb->insts.begin();
        while (it != bb->insts.end()) {
            if (it->name == "phi") {
                bb->insts.erase(it);
            }
            else it++;
        }
    }
}

void removePhi(IR& ir) {
    for (auto f = ir.functions.begin(); f != ir.functions.end(); f++) {
        if (f->name != "@getint" && f->name != "@putint" && f->name != "@putstr") {
            removePhi(*f);
            f->mergeBlk();
            f->divBlk();
        }
    }
}

}