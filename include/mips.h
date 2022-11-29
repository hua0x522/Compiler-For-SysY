#ifndef __MIPS__
#define __MIPS__

#include <vector>
#include <string>
using namespace std;

class Op 
{
public:
    string reg;
    int num;
    Op() {}
    Op(int n): num(n) {}
    Op(string r): reg(r) {}
    string toString();
};

class MInst
{
public:
    string name;
    vector<Op> ops;
    MInst(string, int, ...);
    string toString();
};

class Segment
{
public:
    vector<MInst> insts;
    void add(MInst);
    void print(FILE*);
};

class MIPS 
{
public:
    Segment data;
    Segment text;
    void print(FILE*);
};

#endif 