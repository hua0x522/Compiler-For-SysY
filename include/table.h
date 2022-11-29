#ifndef __TABLE__
#define __TABLE__

#include <vector>
#include <string>
using namespace std;

class Var
{
public:
    string id;
    string tp;
    int dim;
    vector<int> dims;
    int pos;
    string reg;
    Var(string i, string t, int d): id(i), tp(t), dim(d) {}
    vector<int> flat();
};

class Func
{
public:
    string id;
    string tp;
    vector<int> args;
    string reg;
};

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