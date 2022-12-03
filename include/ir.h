#ifndef __IR__
#define __IR__

#include <vector>
#include <stdarg.h>
#include <string>
using namespace std;

string i2s(int);
namespace GEN{
    string newLabel();
}

class Type 
{
public:
    string data;
    vector<int> shape;
    int ptr;
    Type() {}
    Type(string d, vector<int> s, int p): data(d), shape(s), ptr(p) {}
    string toString();
    bool like(Type);
};

/*   value maybe a v-reg or a number   */
class Value
{
public:
    string reg;
    Type ty;
    int num;
    Value() {}
    Value(string r, Type t): reg(r), ty(t) {}
    Value(int n, Type t):num(n), ty(t) {}
    string toString();

    bool operator==(Value& other) {
        return this->toString() == other.toString();
    }
    int getDim() {
        return this->ty.shape.size();
    }
};

class Inst
{
public:
    string name;
    vector<Value> ops;
    Inst() {}
    Inst(string, int cnt, ...);
    Value getVal(string reg);
    string toString();
    string getRes();
};

class Blk
{
public: 
    vector<Inst> insts;
    string name;
    Blk() {}
    Blk(string n) {
        this->name = n;
    }
    void add(Inst i);
    void addHead(Inst i);
    Value getVal(string reg);
};

class Function
{
public:
    vector<Blk> blks;
    vector<Value> args; 
    Value ret;
    string name;
    Function() { blks.push_back(Blk()); }
    void add(Inst i);
    void addBlk(string);
    void clear();
    Value getVal(string reg);
    string toString();
    void checkRet();
    void mergeBlk();
    void divBlk();
    void processBlk();
    int getBlk(string);
};

class IR
{
public:
    Blk global;
    vector<Function> functions;
    void add(Function f);
    Function getFunc(string reg);
    Value getVal(string reg);
};

#endif