#ifndef __CHECK__
#define __CHECK__

#include "node.h"
#include "table.h"
#include <cstdio>
#include <algorithm>

struct Err {
    string err;
    int line;
    Err(string e, int l): err(e), line(l) {}
};

void cCompUnit(Node*);
void cDecl(Node*);
void cConstDecl(Node*);
void cConstDef(Node*);
void cConstInitVal(Node*);
void cConstExp(Node*);
void cAddExp(Node*);
void cMulExp(Node*);
void cUnaryExp(Node*);
void cUnaryOp(Node*);
void cPrimaryExp(Node*);
void cLVal(Node*);
void cNumber(Node*);
void cExp(Node*);
void cVarDecl(Node*);
void cVarDef(Node*);
void cInitVal(Node*);
void cFuncDef(Node*);
void cFuncType(Node*);
void cFuncFParams(Node*);
void cFuncFParam(Node*);
void cBlock(Node*);
void cBlockItem(Node*);
void cStmt(Node*);
void cCond(Node*);
void cLOrExp(Node*);
void cLAndExp(Node*);
void cEqExp(Node*);
void cRelExp(Node*);
void cFuncRParams(Node*);
void cMainFuncDef(Node*);

void err_log(string, int);
void check(Node*);
#endif