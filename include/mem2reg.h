//
// Created by wangxuezhu on 2022/11/28.
//

#ifndef COMPILER_MEM2REG_H
#define COMPILER_MEM2REG_H
#include "generate.h"
#include "graph.h"
#include <set>
#include <map>

void removeBlkAlloc(Blk& block);
void removeFuncAlloc(Function& f);
void mem2reg(IR&);
void addPhi(Value&);
void rename(Value&, int, vector<int>&, vector<Value>&);

#endif //COMPILER_MEM2REG_H
