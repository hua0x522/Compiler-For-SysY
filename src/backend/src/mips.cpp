#include "mips.h"
#include "ir.h"
#include <cstdio>

string Op::toString() {
    if (reg != "") return reg;
    else return i2s(num);
}

MInst::MInst(string n, int cnt, ...) {
    name = n;
    va_list args;
    va_start(args, cnt);
    for (int i = 0; i < cnt; i++) {
        Op op = va_arg(args, Op);
        ops.push_back(op);
    }
}

string MInst::toString() {
    string s;
    if (name == ".space") {
        s = ops[0].reg + ":" + " .space " + i2s(ops[1].num);
    }
    else if (name == "li") {
        s = "li " + ops[0].reg + ", " + i2s(ops[1].num); 
    }
    else if (name == "la") {
        s = "la " + ops[0].reg + ", " + ops[1].reg;
    }
    else if (name == "sw" || name == "lw") {
        s = name + " " + ops[0].reg + ", ";
        if (ops[1].reg == "") s += i2s(ops[1].num);
        else if (ops[1].reg[0] == '$') s += "(" + ops[1].reg + ")";
        else s += ops[1].reg;
        if (ops.size() == 3) s += "(" + ops[2].reg + ")";
    }
    else if (name == "addu" || name == "subu" || name == "mul" || name == "div") {
        s = name + " " + ops[0].reg + ", " + ops[1].reg + ", ";
        if (ops[2].reg == "") s += i2s(ops[2].num);
        else s += ops[2].reg;
    }
    else if (name == "addiu") {
        s = name + " " + ops[0].reg + ", " + ops[1].reg + ", " + i2s(ops[2].num);
    }
    else if (name == "sll") {
        s = name + " " + ops[0].reg + ", " + ops[1].reg + ", " + i2s(ops[2].num);
    }
    else if (name == "mfhi") {
        s = name + " " + ops[0].reg;
    }
    else if (name == "move") {
        s = name + " " + ops[0].reg + ", " + ops[1].reg;
    }
    else if (name == "jal" || name == "j" || name == "jr") {
        s = name + " " + ops[0].reg;
    }
    else if (name == "syscall") {
        s = name;
    }
    else if (name == "label") {
        s = ops[0].reg + ":";
    }
    else if (name == "nop") {
        s = name;
    }
    return s;
}

void Segment::add(MInst i) {
    insts.push_back(i);
}

void Segment::print(FILE* fp) {
    for (int i = 0; i < insts.size(); i++) {
        if (insts[i].name == "label" || insts[i].name == ".space") 
            fprintf(fp, "%s\n", insts[i].toString().c_str());
        else fprintf(fp, "  %s\n", insts[i].toString().c_str());
    }
}

void MIPS::print(FILE* fp) {
    fprintf(fp, ".data\n");
    data.print(fp);
    fprintf(fp, ".text\n");
    text.print(fp);
}