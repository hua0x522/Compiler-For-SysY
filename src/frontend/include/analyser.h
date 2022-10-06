#ifndef __ANALYSER__
#define __ANALYSER__

#include <map>
#include "lexer.h"
#include "node.h"

void LL();

string getType(string);
void initMap();

void CompUnit(Node*, vector<Token>&);
void Decl(Node*, vector<Token>&);
void ConstDecl(Node*, vector<Token>&);
void ConstDef(Node*, vector<Token>&);
void ConstInitVal(Node*, vector<Token>&);
void ConstExp(Node*, vector<Token>&);
void AddExp(Node*, vector<Token>&);
void MulExp(Node*, vector<Token>&);
void UnaryExp(Node*, vector<Token>&);
void UnaryOp(Node*, vector<Token>&);
void PrimaryExp(Node*, vector<Token>&);
void LVal(Node*, vector<Token>&);
void Number(Node*, vector<Token>&);
void Exp(Node*, vector<Token>&);
void VarDecl(Node*, vector<Token>&);
void VarDef(Node*, vector<Token>&);
void InitVal(Node*, vector<Token>&);
void FuncDef(Node*, vector<Token>&);
void FuncType(Node*, vector<Token>&);
void FuncFParams(Node*, vector<Token>&);
void FuncFParam(Node*, vector<Token>&);
void Block(Node*, vector<Token>&);
void BlockItem(Node*, vector<Token>&);
void Stmt(Node*, vector<Token>&);
void Cond(Node*, vector<Token>&);
void LOrExp(Node*, vector<Token>&);
void LAndExp(Node*, vector<Token>&);
void EqExp(Node*, vector<Token>&);
void RelExp(Node*, vector<Token>&);
void FuncRParams(Node*, vector<Token>&);
void MainFuncDef(Node*, vector<Token>&);

void print_tree(Node*, FILE*);

#endif