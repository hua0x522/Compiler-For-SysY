#ifndef __TABLE__
#define __TABLE__
#include "var.h"
#include "func.h"

class Table
{
public:
    vector<Var> vars;
    vector<Func> funcs;
    bool has(string);
    void add(Var); 
    void add(Func);
    void inScope();
    void outScope();
    Var findVar(string);
    Func findFunc(string);
    void toString();
};

#endif