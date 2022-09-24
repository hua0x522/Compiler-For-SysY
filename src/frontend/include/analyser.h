#ifndef __ANALYSER__
#define __ANALYSER__

#include <map>
#include "lexer.h"
#include "node.h"

void LL();

string getType(string);
void initMap();

bool CompUnit(Node*, vector<string>&);
bool Decl(Node*, vector<string>&);
bool ConstDecl(Node*, vector<string>&);
bool ConstDef(Node*, vector<string>&);
bool ConstInitVal(Node*, vector<string>&);
bool ConstExp(Node*, vector<string>&);
bool AddExp(Node*, vector<string>&);
bool MulExp(Node*, vector<string>&);
bool UnaryExp(Node*, vector<string>&);
bool UnaryOp(Node*, vector<string>&);
bool PrimaryExp(Node*, vector<string>&);
bool LVal(Node*, vector<string>&);
bool Number(Node*, vector<string>&);
bool Exp(Node*, vector<string>&);
bool VarDecl(Node*, vector<string>&);
bool VarDef(Node*, vector<string>&);
bool InitVal(Node*, vector<string>&);
bool FuncDef(Node*, vector<string>&);
bool FuncType(Node*, vector<string>&);
bool FuncFParams(Node*, vector<string>&);
bool FuncFParam(Node*, vector<string>&);
bool Block(Node*, vector<string>&);
bool BlockItem(Node*, vector<string>&);
bool Stmt(Node*, vector<string>&);
bool Cond(Node*, vector<string>&);
bool LOrExp(Node*, vector<string>&);
bool LAndExp(Node*, vector<string>&);
bool EqExp(Node*, vector<string>&);
bool RelExp(Node*, vector<string>&);
bool FuncRParams(Node*, vector<string>&);
bool MainFuncDef(Node*, vector<string>&);

void print_tree(Node*, FILE*);

#endif