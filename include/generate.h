#ifndef __GENERATE__
#define __GENERATE__

#include "node.h"
#include "table.h"
#include "ir.h"
#include <vector>
#include <cstdio>
using namespace std;

namespace GEN
{
string newReg();
void gCompUnit(Node*);
void gDecl(Node*);
void gConstDecl(Node*);
void gConstDef(Node*);
void gConstInitVal(Node*);
void gVarDecl(Node*);
void gVarDef(Node*);
void gInitVal(Node*);
void gFuncDef(Node*);
void gMainFuncDef(Node*);
void gFuncType(Node*);
void gFuncFParams(Node*);
void gFuncFParam(Node*);
void gBlock(Node*);
void gBlockItem(Node*);
void gStmt(Node*);
Value gPrimaryExp(Node*);
Value gNumber(Node*);
Value gExp(Node*);
Value gLVal(Node*, bool);
Value gUnaryExp(Node*);
vector<Value> gFuncRParams(Node*);
Value gMulExp(Node*);
Value gAddExp(Node*);
void gCond(Node*, string, string);
void gLOrExp(Node*, string, string); 
void gLAndExp(Node*, string, string);
Value gEqExp(Node*);
Value gRelExp(Node*);
IR generate(Node*);

void putstr(string&);
void i32toi1(Value&);
void i1toi32(Value&);
/*    tools region     */

int conAddExp(Node*);
int conMulExp(Node*);
int conUnaryExp(Node*);
int conPrimaryExp(Node*);
int conNumber(Node*);
int conExp(Node*);
int conLVal(Node*);
int gConstExp(Node*);

void printIR(IR&, FILE*);
void printFunction(Function&, FILE*);
void printBlk(Blk&, FILE*);
}
#endif