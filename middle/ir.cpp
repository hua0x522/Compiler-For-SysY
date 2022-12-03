#include "ir.h"

string i2s(int num) {
    string s;
    if (!num) s = "0";
    int flag = 0;
    if (num < 0) {
        flag = 1;
        num = -num;
    }
    while (num) {
        s = string(1, '0'+(num%10)) + s;
        num = num / 10;
    }
    if (flag) s = "-" + s;
    return s;
}

string Type::toString() {
    string s;
    if (!shape.size()) s = data;
    else {
        int num = 1;
        for (int i = 0; i < shape.size(); i++) num *= shape[i];
        s = "[" + i2s(num) + " x " + data +"]";
    }
    s += string(ptr, '*');
    return s;
}

bool Type::like(Type other) {
    if (data != other.data) return false;
    if (shape.size() != other.shape.size()) return false;
    for (int i = 0; i < shape.size(); i++) {
        if (shape[i] != other.shape[i]) return false;
    }
    if (ptr != other.ptr) return false;
    return true;
}

string Value::toString() {
    if (reg.size()) return reg;
    else return i2s(num);
}

Inst::Inst(string n, int cnt, ...) {
    name = n;
    va_list args;
    va_start(args, cnt);
    for (int i = 0; i < cnt; i++) {
        Value v = va_arg(args, Value);
        ops.push_back(v);
    }
}

Value Inst::getVal(string reg) {
    for (int i = 0; i < ops.size(); i++) {         
        if (ops[i].reg == reg) return ops[i];
    }
    return Value();
}

string Inst::toString() {
    string s;
    if (name == "global") {
        s += ops[0].toString() + " = global ";
        Type ty = ops[0].ty;
        ty.ptr--;
        s += ty.toString();
        if (ops.size() > 1) {
            if (ops[0].ty.shape.size() > 0) {
                s += " [";
                for (int i = 1; i < ops.size(); i++) {
                    if (i != 1) s += ", ";
                    s += ops[i].ty.toString() + " " + ops[i].toString();
                }
                s += "]";
            }
            else {
                s += " " + ops[1].toString();
            }
        } 
        else {
            s += " zeroinitializer";
        }
    }
    else if (name == "constant") {
        s += ops[0].toString() + " = constant ";
        Type ty = ops[0].ty;
        ty.ptr--;
        s += ty.toString();
        if (ops[0].ty.data == "i8") {
            s += " c\"";
            for (int i = 0; i < ops[1].reg.size(); i++) {
                if (ops[1].reg[i] == '\n') {
                    s += "\\0a";
                }
                else s += string(1, ops[1].reg[i]);
            }
            s += "\\00\"";
        }
        else if (ops[0].ty.shape.size() > 0) {
            s += " [";
            for (int i = 1; i < ops.size(); i++) {
                if (i != 1) s += ", ";
                s += ops[i].ty.toString() + " " + ops[i].toString();
            }
            s += "]";
        }
        else {
            s += " " + ops[1].toString();
        }
    }
    else if (name == "add") {
        s += ops[0].toString() + " = add " + ops[0].ty.toString() + " " +
            ops[1].toString() + ", " + ops[2].toString();
    }
    else if (name == "sub") {
        s += ops[0].toString() + " = sub " + ops[0].ty.toString() + " " +
            ops[1].toString() + ", " + ops[2].toString();
    }
    else if (name == "mul") {
        s += ops[0].toString() + " = mul " + ops[0].ty.toString() + " " +
            ops[1].toString() + ", " + ops[2].toString();
    }
    else if (name == "sdiv") {
        s += ops[0].toString() + " = sdiv " + ops[0].ty.toString() + " " +
            ops[1].toString() + ", " + ops[2].toString();
    }
    else if (name == "srem") {
        s += ops[0].toString() + " = srem " + ops[0].ty.toString() + " " +
            ops[1].toString() + ", " + ops[2].toString();
    }
    else if (name == "call") {
        auto it = ops.begin();
        if ((*it).reg[0] != '@') {  // have return, must use a temp vreg to get the return val
            s += (*it).toString() + " = ";
            it++;
        }
        s += "call " + (*it).ty.toString() + " " + (*it).toString();
        it++;
        s += "(";
        while (it != ops.end()) {
            s += (*it).ty.toString() + " " + (*it).toString();
            it++;
            if (it != ops.end()) s += ", ";
        }
        s += ")";
    }
    else if (name == "alloca") {
        Type ty = ops[0].ty;
        ty.ptr--;
        s += ops[0].toString() + " = alloca " + ty.toString();
    }
    else if (name == "load") {
        s += ops[0].toString() + " = load " + ops[0].ty.toString() + ", " + 
        ops[1].ty.toString() + " " + ops[1].toString(); 
    }
    else if (name == "store") {
        s += "store " + ops[0].ty.toString() + " " + ops[0].toString() + ", " + 
        ops[1].ty.toString() + " " + ops[1].toString();
    }
    else if (name == "getelementptr inbounds") {
        Type ty = ops[1].ty;
        ty.ptr--;
        s += ops[0].toString() + " = getelementptr inbounds " + ty.toString() + ", " +
        ops[1].ty.toString() + " " + ops[1].toString();
        for (int i = 2; i < ops.size(); i++) {
            s += ", ";
            s += ops[i].ty.toString() + " " + ops[i].toString();
        }
    }
    else if (name == "ret") {
        s += "ret " + ops[0].ty.toString();
        if (ops[0].ty.toString() != "void") s += " " + ops[0].toString();
    }
    else if (name == "label") {
        s += ops[0].toString() + ":";
    }
    else if (name == "br") {
        if (ops.size() == 3) {
            s += name + " " + ops[0].ty.toString() + " " + ops[0].toString() + ", ";
            s += "label %" + ops[1].toString() + ", label %" + ops[2].toString(); 
        }
        else {
            s += name + " label %" + ops[0].toString();
        }
    }
    else if (name == "icmp") {
        s += ops[0].toString() + " = icmp " + ops[1].toString() + " " + ops[2].ty.toString() +
        " " + ops[2].toString() + ", " + ops[3].toString();
    }
    else if (name == "zext") {
        s += ops[0].toString() + " = zext " + ops[1].ty.toString() + " " + ops[1].toString() + 
        " to " + ops[0].ty.toString();
    }
    else if (name == "phi") {
        s += ops[0].toString() + " = phi " + ops[0].ty.toString() + " ";
        for (int i = 1; i < ops.size(); i += 2) {
            if (i != 1) s += ", ";
            s += "[" + ops[i].toString() + ", %" + ops[i+1].toString() + "]";
        }
    }
    else if (name == "move") {
        s += "move " + ops[0].toString() + " to " + ops[1].toString();
    }
    return s;
}

string Inst::getRes() {
    if (name == "global" || name == "constant" || name == "alloca" || name == "add" || name == "sub" ||
        name == "mul" || name == "sdiv" || name == "srem" || name == "load" || name == "getelementptr inbounds" ||
        name == "icmp" || name == "zext") {
        return ops[0].toString();
    }
    if (name == "move") {
        return ops[1].toString();
    }
    return "";
}

void Blk::add(Inst i) { 
    insts.push_back(i); 
}

void Blk::addHead(Inst i) {
    // begin is label
    insts.insert(insts.begin()+1, i);
}

Value Blk::getVal(string reg) {
    for (int i = 0; i < insts.size(); i++) {
        Value val = insts[i].getVal(reg);
        if (val.reg == reg) return val;
    }
    return Value();
}

void Function::add(Inst i) {
    (*(blks.end()-1)).add(i);
}

void Function::addBlk(string n) {
    blks.push_back(Blk(n));
}

void Function::clear() {
    blks.clear();
    blks.push_back(Blk());
    args.clear();
    ret = Value();
}

Value Function::getVal(string reg) {
    for (int i = 0; i < blks.size(); i++) {
        Value val = blks[i].getVal(reg);
        if (val.reg == reg) return val;
    }
    return Value();
}

string Function::toString() {
    string s = "define " + ret.ty.toString() + " " + ret.toString();
    s += "(";
    for (int i = 0; i < args.size(); i++) {
        if (i) s += ", ";
        s += args[i].ty.toString() + " " + args[i].toString();
    }
    s += ")";
    return s;
}

int Function::getBlk(string n) {
    for (int i = 0; i < blks.size(); i++) {
        if (blks[i].name == n) return i;
    }
    return -1;
}

void Function::checkRet() {
    if (ret.ty.toString() != "void") return;
    if (blks[blks.size()-1].insts.size() > 0) {
        Inst inst = *(blks[blks.size()-1].insts.end()-1);
        if (inst.name == "ret") return;
    }
    Type ty("void", vector<int>(), 0);
    Value v(0, ty);
    Inst inst("ret", 1, v);
    add(inst);
}

void Function::mergeBlk() {
    for (auto bb = blks.begin()+1; bb != blks.end(); bb++) {
        for (auto it = bb->insts.begin(); it != bb->insts.end(); it++) {
            blks[0].add(*it);
        }
    }
    while (blks.size() > 1) {
        blks.pop_back();
    }
}

void Function::divBlk() {
    int BB0end = 0;
    for (int i = 1; i < blks[0].insts.size(); i++) {
        if (blks[0].insts[i].name == "label") {
            if (!BB0end) BB0end = i;
            addBlk(blks[0].insts[i].ops[0].toString());
        }
        if (BB0end) add(blks[0].insts[i]);
    }
    if (!BB0end) return;
    while (blks[0].insts.size() > BB0end) {
        blks[0].insts.pop_back();
    }
}

void Function::processBlk() {
    if (blks[0].insts[0].name == "label") {
        Inst br("br", 1, blks[0].insts[0].ops[0]);
        blks[0].insts.insert(blks[0].insts.begin(), br);
    }
    string l = GEN::newLabel();
    Inst label("label", 1, Value(l, Type()));
    blks[0].insts.insert(blks[0].insts.begin(), label);
    blks[0].name = l;

    divBlk();

    // move alloca to begin of the first blk
    vector<Inst> temp;
    for (int i = 0; i < blks.size(); i++) {
        auto j = blks[i].insts.begin();
        while (j != blks[i].insts.end()) {
            if ((*(j)).name == "alloca") {
                Inst inst = *(j);
                blks[i].insts.erase(j);
                temp.push_back(inst);
            }
            else j++;
        }
    }
    for (int i = 0; i < temp.size(); i++) {
        blks[0].insts.insert(blks[0].insts.begin()+1, temp[i]);
    }

    // to ensure each blk end of ret/br
    for (int i = 0; i < blks.size(); i++) {
        if (i == blks.size()-1) break;
        Inst i1 = blks[i].insts[blks[i].insts.size()-1];
        Inst i2 = blks[i+1].insts[0];
        if (i1.name != "ret" && i1.name != "br") {
            Inst inst("br", 1, i2.ops[0]);
            blks[i].add(inst);
        }
    }

    // to ensure only one ret/br in a blk
    for (int i = 0; i < blks.size(); i++) {
        bool isEnd = false;
        auto it = blks[i].insts.begin();
        while (it != blks[i].insts.end()) {
            if (isEnd) blks[i].insts.erase(it);
            else {
                if (it->name == "ret" || it->name == "br") isEnd = true;
                it++;
            }
        }
    }
}

void IR::add(Function f) {
    functions.push_back(f);
}

Function IR::getFunc(string reg) {
    for (int i = 0; i < functions.size(); i++) {
        if (functions[i].ret.reg == reg)
            return functions[i];
    }
    return Function();
}

Value IR::getVal(string reg) {
    Value val = global.getVal(reg);
    if (val.reg == reg) return val;
    for (int i = 0; i < functions.size(); i++) {
        Value v = functions[i].getVal(reg);
        if (v.reg == reg) return v;
    }
    return Value();
}