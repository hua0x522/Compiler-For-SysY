//
// Created by wangxuezhu on 2022/12/2.
//

#include "mem2reg.h"

extern Function* f;
map<string, int> constants;
set<string> binary = {"add", "sub", "mul", "sdiv", "srem"};

void constantPropagation(IR& ir) {
    auto it = ir.global.insts.begin();
    while (it != ir.global.insts.end()) {
        if (it->name == "constant" && it->ops[0].ty.shape.size() == 0) {
            constants.insert({it->ops[0].toString(), it->ops[1].num});
            ir.global.insts.erase(it);
        }
        else it++;
    }
    for (auto func = ir.functions.begin(); func != ir.functions.end(); func++) {
        f = &(*func);
        for (auto bb = func->blks.begin(); bb != func->blks.end(); bb++) {
            it = bb->insts.begin();
            while (it != bb->insts.end()) {
                if (it->name == "load" && constants.find(it->ops[1].toString()) != constants.end()) {
                    replaceValue(it->ops[0], Value(constants[it->ops[1].toString()], it->ops[0].ty));
                    bb->insts.erase(it);
                }
                else it++;
            }
        }
    }
}

