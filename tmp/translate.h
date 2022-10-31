#ifndef __TRANSLATE__
#define __TRANSLATE__

#include "mips.h"
#include "ir.h"
#include <cstdio>
#include <map>

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
    Pool(int cnt, ...) {
        va_list args;
        va_start(args, cnt);
        for (int i = 0; i < cnt; i++) {
            string s = va_arg(args, const char*);
            Reg reg = Reg(s, "");
            regs.push_back(reg);
        }
    }
    string alloc(string vreg) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].use == "") {
                regs[i].use = vreg;
                return regs[i].id;
            }
        }
        printf("Regs Full\n");
        return "";
    }
    string query(string vreg) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].use == vreg) {
                return regs[i].id;
            }
        }
        printf("Query Fault\n");
        return "";
    }
    void free(string id) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].id == id) {
                regs[i].use = "";
            }
        }
    }
};

string val2reg(Value);
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
void putch(Inst&);
void transInst(Inst&);
void transBlk(Blk&, int);
void transFunc(Function&);
void transIR(IR&);
void translate(IR&);

}

#endif