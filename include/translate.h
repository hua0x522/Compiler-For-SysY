#ifndef __TRANSLATE__
#define __TRANSLATE__

#include "mips.h"
#include "ir.h"
#include <cstdio>
#include <map>
#include <set>

namespace TRANS
{

class Reg
{
public:
    string id;
    string use;
    Reg(string i, string u): id(i), use(u) {}
};

class Pool
{
public:
    vector<Reg> regs;
    vector<int> ages;
    int counter = 0;
    Pool(int cnt, ...) {
        va_list args;
        va_start(args, cnt);
        for (int i = 0; i < cnt; i++) {
            string s = va_arg(args, const char*);
            Reg reg = Reg(s, "");
            regs.push_back(reg);
            ages.push_back(0);
        }
    }
    string alloc(string vreg);
    void clean();
    string query(string vreg) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].use == vreg) {
                return regs[i].id;
            }
        }
        return "";
    }
    void free(string id) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].id == id) {
                regs[i].use = "";
            }
        }
    }
    void replace(string v1, string v2) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].use == v1) {
                regs[i].use = v2;
                ages[i] = 0;
            }
        }
    }
    void reset() {
        for (int i = 0; i < regs.size(); i++) {
            regs[i].use = "";
            ages[i] = counter;
        }
    }
};

string val2reg(Value, bool);
string getAddr(Value);
void global_data(Inst&);
void local_data(Inst&);
void binary(Inst&);
void load(Inst&);
void store(Inst&);
void getelementptr(Inst&);
void save();
int call(Inst&);
void restore(int, Inst&);
void ret(Inst&);
void getint(Inst&);
void putint(Inst&);
void putstr(Inst&);
void transInst(Inst&);
void transBlk(Blk&, int);
void transFunc(Function&);
void transIR(IR&);
void translate(IR&);

bool useful(string, int, int, vector<int>&);
}

#endif