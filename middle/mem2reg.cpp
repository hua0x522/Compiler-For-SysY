//
// Created by wangxuezhu on 2022/11/28.
//
#include "mem2reg.h"

Graph* graph;
Function* f;
set<int> DF;
Type i32("i32", vector<int>(), 0);
void removeUnreach() {
    for (int i = f->blks.size() - 1; i >= 0; i--) {
        if (graph->reach[i] == 0) {
            f->blks.erase(f->blks.begin() + i);
        }
    }
}

void mem2reg(IR& ir) {
    for (int i = 0; i < ir.functions.size(); i++) {
        removeFuncAlloc(ir.functions[i]);
    }
    FILE* fp = fopen("llvm_ir.txt", "w");
    GEN::printIR(ir, fp);
    fclose(fp);
    GEN::removePhi(ir);
    constantPropagation(ir);
    fp = fopen("mem2reg.txt", "w");
    GEN::printIR(ir, fp);
    fclose(fp);
}

void removeFuncAlloc(Function& func) {
    if (func.ret.reg == "@getint" || func.ret.reg == "@putint" || func.ret.reg == "@putstr") return;
    f = &func;
    graph = new Graph(func.blks.size());
    for (int i = 0; i < func.blks.size(); i++) {
        for (int j = 0; j < func.blks[i].insts.size(); j++) {
            Inst inst = func.blks[i].insts[j];
            if (inst.name == "ret") break;
            else if (inst.name == "br") {
                if (inst.ops.size() == 1) {
                    string tar = inst.ops[0].toString();
                    int v = func.getBlk(tar);
                    graph->add(i, v);
                }
                else {
                    string tar1 = inst.ops[1].toString(), tar2 = inst.ops[2].toString();
                    int v1 = func.getBlk(tar1), v2 = func.getBlk(tar2);
                    graph->add(i, v1);
                    graph->add(i, v2);
                }
                break;
            }
        }
    }
    graph->init();
    for (int i = 0; i < func.blks.size(); i++) {
        removeBlkAlloc(func.blks[i]);
    }
    removeUnreach();
}

void removeBlkAlloc(Blk& block) {
    auto it = block.insts.begin();
    while (it != block.insts.end()) {
        if (it->name == "alloca" && it->ops[0].getDim() == 0) {
            addPhi(it->ops[0]);
            vector<int> vis(f->blks.size(), 0);
            vector<Value> avatars(f->blks.size(), Value(0, i32));
            rename(it->ops[0], 0, vis, avatars);
            block.insts.erase(it);
        }
        else it++;
    }
}

void addPhi(Value& x) {
    Type ty = x.ty;
    ty.ptr--;
    DF.clear();
    int defs = 0;
    for (auto bb = f->blks.begin(); bb != f->blks.end(); bb++) {
        for (auto it = bb->insts.begin(); it != bb->insts.end(); it++) {
            if (it->name == "store" && it->ops[1] == x) {
                defs++;
                int idx = bb - f->blks.begin();
                for (auto df = graph->iterDF[idx].begin(); df != graph->iterDF[idx].end(); df++) {
                    DF.insert(*df);
                }
            }
        }
    }
    if (defs == 1) DF.clear(); //one def point don't have Phi
    for (auto it = DF.begin(); it != DF.end(); it++) {
        Inst phi("phi", 1, Value(GEN::newReg(), ty));
        f->blks[*it].addHead(phi);
    }
}

void replaceValue(Value oldVal, Value newVal) {
    for (auto bb = f->blks.begin(); bb != f->blks.end(); bb++) {
        for (auto it = bb->insts.begin(); it != bb->insts.end(); it++) {
            for (auto op = it->ops.begin(); op != it->ops.end(); op++) {
                if (*op == oldVal) {
                    *op = newVal;
                }
            }
        }
    }
}

void rename(Value& x, int u, vector<int>& vis, vector<Value>& avatars) {
    Value avatar = avatars[u];
    vis[u] = 1;
    auto it = f->blks[u].insts.begin();
    bool usePhi = false;
    while (it != f->blks[u].insts.end()) {
        if (it->name == "phi" && DF.find(u) != DF.end() && !usePhi) {
            usePhi = true;
            avatar = it->ops[0];
            it++;
        }
        else if (it->name == "load" && it->ops[1] == x) {
            avatar.ty = it->ops[0].ty;
            replaceValue(it->ops[0], avatar);
            f->blks[u].insts.erase(it);
        }
        else if (it->name == "store" && it->ops[1] == x) {
            avatar = it->ops[0];
            f->blks[u].insts.erase(it);
        }
        else it++;
    }
    for (int i = 0; i < graph->edge[u].size(); i++) {
        int v = graph->edge[u][i];
        if (DF.find(v) != DF.end()) {
            Inst* ptr = &f->blks[v].insts[1];
            Type ty = avatar.ty;
            ptr->ops.push_back(avatar);
            ptr->ops.push_back(Value(f->blks[u].name, ty));
        }
        else avatars[v] = avatar;
    }
    for (int i = 0; i < graph->edge[u].size(); i++) {
        int v = graph->edge[u][i];
        if (!vis[v]) rename(x, v, vis, avatars);
    }
}