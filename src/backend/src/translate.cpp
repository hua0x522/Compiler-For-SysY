#include "translate.h"

namespace TRANS
{

MIPS mips;
Segment data;
Segment text;
vector<string> saved;
map<string, int> table;
bool isMain;
// Pool pool(24, "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9", "$a1", "$a2", "$a3",
//               "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$v1", "$k0", "$k1");
Pool pool(4, "$t0", "$t1", "$t2", "$t3");

string Pool::alloc(string vreg) {
    int idx = -1;
    for (int i = 0; i < regs.size(); i++) {
        if (regs[i].use == "") {
            regs[i].use = vreg;
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = 0; i < regs.size(); i++) {
            if (i == idx) ages[i] = 0;
            else ages[i] += 1;
        }
        return regs[idx].id;
    }
    else {
        int age = 0;
        for (int i = 0; i < regs.size(); i++) {
            if (ages[i] >= age) {
                age = ages[i];
                idx = i;
            }
        }
        MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(-4));
        text.add(addiu);
        MInst sw("sw", 3, Op(regs[idx].id), Op(0), Op("$sp"));
        text.add(sw);
        text.add(MInst("//", 1, Op("### " + regs[idx].id + " overflow, " + 
            regs[idx].use + " give to " + vreg)));
        table["$sp"]  -= 4;
        table.insert({regs[idx].use, table["$sp"]});
        overflow.push_back(regs[idx].use);
        regs[idx].use = vreg;
        string id = regs[idx].id;
        return id;
    }
}

void Pool::clean() {
    if (!overflow.size()) return;
    MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(4*overflow.size()));
    text.add(addiu);
    table["$sp"]  += 4 * overflow.size();
    for (int i = 0; i < overflow.size(); i++) {
        table.erase(overflow[i]);
    }
    overflow.clear();
}

string val2reg(Value v) {
    string reg;
    if (v.reg == "") {
        reg = pool.alloc(i2s(v.num));
        MInst li("li", 2, Op(reg), Op(v.num));
        text.add(li);
    }
    else if (v.reg[0] == '@') {
        reg = pool.alloc(v.reg);
        string addr = string(v.reg.begin()+1, v.reg.end());
        MInst la("la", 2, Op(reg), Op(addr));
        text.add(la); 
    }
    else if (table.find(v.reg) == table.end()) {
        reg = pool.query(v.reg);
    }
    else {
        reg = pool.alloc(v.reg);
        int offset = table[v.reg] - table["$sp"];
        MInst lw("lw", 3, Op(reg), Op(offset), Op("$sp"));
        text.add(lw);
    }
    return reg;
}

string val2reg_addr(Value v) {
    if (pool.isOverflow(v.reg)) {
        return val2reg(v);
    }
    string reg;
    if (v.reg[0] == '@') { 
        reg = pool.alloc(v.reg);
        string addr = string(v.reg.begin()+1, v.reg.end());
        MInst la("la", 2, Op(reg), Op(addr));
        text.add(la); 
    }
    else if (table.find(v.reg) == table.end()) {
        reg = pool.query(v.reg);
    }
    else {
        reg = pool.alloc(v.reg);
        int offset = table[v.reg] - table["$sp"];
        MInst addiu("addiu", 3, Op(reg), Op("$sp"), Op(offset));
        text.add(addiu); 
    }
    return reg;
}

void global_data(Inst& inst) {
    string label = string(inst.ops[0].reg.begin()+1, inst.ops[0].reg.end());
    if (inst.ops[0].ty.data == "i8") {
        MInst m(".asciiz", 2, Op(label), Op(inst.ops[1].reg));
        data.add(m);
        return;
    }
    int size = (inst.ops[0].ty.shape.size() == 0) ? 1 : inst.ops[0].ty.shape[0];
    MInst m(".space", 2, Op(label), Op(4*size));
    data.add(m);
    for (int i = 1; i < inst.ops.size(); i++) {
        string reg = pool.alloc(inst.ops[i].toString());
        MInst li("li", 2, Op(reg), Op(inst.ops[i].num));
        text.add(li);
        string addr = pool.alloc(label);
        MInst la("la", 2, Op(addr), Op(label));
        text.add(la);
        MInst sw("sw", 3, Op(reg), Op((i-1)*4), Op(addr));
        text.add(sw);
        pool.free(addr);
        pool.free(reg);
    }
}

void local_data(Inst& inst) {
    if (table.find(inst.ops[0].reg) != table.end()) return;
    /*   if size != 0, op must be not a ptr (we only have 1dim arr)  */
    int size = (inst.ops[0].ty.shape.size() == 0) ? 1 : inst.ops[0].ty.shape[0];    
    MInst m("addiu", 3, Op("$sp"), Op("$sp"), Op(-4*size));
    text.add(m);
    table["$sp"]  -= 4 * size;
    table.insert({inst.ops[0].reg, table["$sp"]});
}

void binary(Inst& inst) {
    string res = pool.alloc(inst.ops[0].toString());
    if (inst.ops[1].reg == "" && inst.ops[2].reg == "") {
        int num = (inst.name == "add") ? inst.ops[1].num + inst.ops[2].num : 
                  (inst.name == "sub") ? inst.ops[1].num - inst.ops[2].num :
                  (inst.name == "mul") ? inst.ops[1].num * inst.ops[2].num : 
                  (inst.name == "sdiv")? inst.ops[1].num / inst.ops[2].num :
                  (inst.name == "srem")? inst.ops[1].num % inst.ops[2].num : 0;
        MInst li("li", 2, Op(res), Op(num));
        text.add(li);
    }
    else {
        string reg1 = val2reg(inst.ops[1]);
        string reg2 = val2reg(inst.ops[2]);
        string name = (inst.name == "add") ? "addu" :
                      (inst.name == "sub") ? "subu" :
                      (inst.name == "mul") ? "mul"   :
                      (inst.name == "sdiv")? "div"   :
                      (inst.name == "srem")? "div"   : 0;
        MInst m(name, 3, Op(res), Op(reg1), Op(reg2));
        text.add(m);
        pool.free(reg1);
        pool.free(reg2);
        if (inst.name == "srem") {
            MInst m("mfhi", 1, Op(res));
            text.add(m);
        }
    }
}

void load(Inst& inst) {
    string res = pool.alloc(inst.ops[0].reg);
    if (inst.ops[1].reg[0] == '@') {
        string addr = string(inst.ops[1].reg.begin()+1, inst.ops[1].reg.end());
        MInst lw("lw", 2, Op(res), Op(addr));
        text.add(lw);
    }
    else if (table.find(inst.ops[1].reg) == table.end()) {
        string addr = pool.query(inst.ops[1].reg);
        MInst lw("lw", 2, Op(res), Op(addr));
        text.add(lw);
        pool.free(addr);
    }
    else {
        int offset = table[inst.ops[1].reg] - table["$sp"];
        MInst lw("lw", 3, Op(res), Op(offset), Op("$sp"));
        text.add(lw);
    }
}

void store(Inst& inst) {
    string val = val2reg(inst.ops[0]);
    if (inst.ops[1].reg[0] == '@') {
        string addr = string(inst.ops[1].reg.begin()+1, inst.ops[1].reg.end());
        MInst sw("sw", 2, Op(val), Op(addr));
        text.add(sw);
    }
    else if (table.find(inst.ops[1].reg) == table.end()) {
        string addr = pool.query(inst.ops[1].reg);
        MInst sw("sw", 2, Op(val), Op(addr));
        text.add(sw);
        pool.free(addr);
    }
    else {
        int offset = table[inst.ops[1].reg] - table["$sp"];
        MInst sw("sw", 3, Op(val), Op(offset), Op("$sp"));
        text.add(sw);
    }
    pool.free(val);
}

void icmp(Inst& inst) {
    map<string, string> imap = {
        {"eq", "seq"}, {"ne", "sne"}, {"sgt", "sgt"}, 
        {"sge", "sge"}, {"slt", "slt"}, {"sle", "sle"}
    };
    string res = pool.alloc(inst.ops[0].reg);
    string tp = inst.ops[1].reg;
    string v1 = val2reg(inst.ops[2]);
    string v2 = val2reg(inst.ops[3]);
    MInst m(imap[tp], 3, Op(res), Op(v1), Op(v2));
    text.add(m);
    pool.free(v1);
    pool.free(v2);
}

void br(Inst& inst) {
    if (inst.ops.size() == 1) {
        MInst j("j", 1, Op(inst.ops[0].reg));
        text.add(j);
    }
    else {
        string v = val2reg(inst.ops[0]);
        string l1 = inst.ops[1].reg;
        string l2 = inst.ops[2].reg;
        MInst bgtz("bgtz", 2, Op(v), Op(l1));
        text.add(bgtz);
        MInst beqz("beqz", 2, Op(v), Op(l2));
        text.add(beqz);
        pool.free(v);
    }
}

void getelementptr(Inst& inst) {    
    string res = pool.alloc(inst.ops[0].toString());
    string addr = val2reg_addr(inst.ops[1]);
    int idx = inst.ops.size()-1;
    if (inst.ops[idx].reg == "") {
        MInst addiu("addiu", 3, Op(res), Op(addr), Op(4*inst.ops[idx].num));
        text.add(addiu);
        pool.free(addr);
    }
    else {
        string reg = pool.query(inst.ops[idx].reg);
        MInst sll("sll", 3, Op(reg), Op(reg), Op(2));
        text.add(sll);
        MInst addu("addu", 3, Op(res), Op(addr), Op(reg));
        text.add(addu);
        pool.free(addr);
        pool.free(reg);
    }
}

void save() {
    for (auto it = pool.regs.begin(); it != pool.regs.end(); it++) {
        if ((*it).use != "") {
            saved.push_back((*it).id);
            (*it).use == "";
        }
    }
    saved.push_back("$ra");
    MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(-4*saved.size()));
    text.add(addiu);
    table["$sp"] -= 4 * saved.size();
    for (int i = 0; i < saved.size(); i++) {
        MInst sw("sw", 3, Op(saved[i]), Op(4*i), Op("$sp"));
        text.add(sw);
    }
}

int call(Inst& inst) {
    int begin = (inst.ops[0].reg[0] == '@') ? 1 : 2;
    int num = inst.ops.size() - begin;
    if (num > 0) {
        MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(-4* num));
        text.add(addiu);
        table["$sp"] -= 4 * num;
        int cnt = 0;
        for (int i = begin; i < inst.ops.size(); i++) {
            string reg = (inst.ops[i].ty.ptr) ? val2reg_addr(inst.ops[i]) : val2reg(inst.ops[i]);
            MInst sw("sw", 3, Op(reg), Op(4*cnt), Op("$sp")); 
            text.add(sw);
            pool.free(reg);
            cnt++;
        }
    } 
    string label = string(inst.ops[begin-1].reg.begin()+1, inst.ops[begin-1].reg.end());
    MInst jal("jal", 1, Op(label));
    text.add(jal);
    MInst nop("nop", 0);
    text.add(nop);
    return 4*num;
}

void restore(int space, Inst& inst) {
    if (space > 0) {
        MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(space));
        table["$sp"] += space;
        text.add(addiu);
    }
    for (int i = 0; i < saved.size(); i++) {
        MInst lw("lw", 3, Op(saved[i]), Op(4*i), Op("$sp"));
        text.add(lw);
    }
    MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(4*saved.size()));
    text.add(addiu);
    table["$sp"] += 4 * saved.size();
    saved.clear();
    if (inst.ops[0].reg[0] != '@') {
        string reg = pool.alloc(inst.ops[0].reg);
        MInst move("move", 2, Op(reg), Op("$v0"));
        text.add(move);
    }
}

void ret(Inst& inst) {
    if (isMain) {
        MInst li("li", 2, Op("$v0"), Op(10));
        text.add(li);
        MInst m("syscall", 0);
        text.add(m);
        return;
    }
    if (inst.ops[0].ty.data != "void") {
        if (inst.ops[0].reg == "") {
            MInst li("li", 2, Op("$v0"), Op(inst.ops[0].num));
            text.add(li);
        }
        else {
            string reg = val2reg(inst.ops[0]);
            MInst move("move", 2, Op("$v0"), Op(reg));
            text.add(move);
            pool.free(reg);   
        }
    }
    MInst jr("jr", 1, Op("$ra"));
    text.add(jr);
    MInst nop("nop", 0);
    text.add(nop);
}

void getint(Inst& inst) {
    MInst li("li", 2, Op("$v0"), Op(5));
    text.add(li);
    MInst syscall("syscall", 0);
    text.add(syscall);
    string res = pool.alloc(inst.ops[0].reg);
    MInst move("move", 2, Op(res), Op("$v0"));
    text.add(move);
}

void putint(Inst& inst) {
    string reg = val2reg(inst.ops[1]);
    MInst move("move", 2, Op("$a0"), Op(reg));
    text.add(move);
    MInst li("li", 2, Op("$v0"), Op(1));
    text.add(li);
    MInst syscall("syscall", 0);
    text.add(syscall);
    pool.free(reg);
}

void putstr(Inst& inst) {
    string reg = val2reg(inst.ops[1]);
    MInst move("move", 2, Op("$a0"), Op(reg));
    text.add(move);
    MInst li("li", 2, Op("$v0"), Op(4));
    text.add(li);
    MInst syscall("syscall", 0);
    text.add(syscall);
    pool.free(reg);
}

void transInst(Inst& inst) {
    if (inst.name == "global" || inst.name == "constant") {
        global_data(inst);
    }
    else if (inst.name == "alloca") {
        local_data(inst);
    }
    else if (inst.name == "add" || inst.name == "sub" || inst.name == "mul" ||
             inst.name == "sdiv" || inst.name == "srem") {
        binary(inst);
    }
    else if (inst.name == "load") {
        load(inst);
    }
    else if (inst.name == "store") {
        store(inst);
    }
    else if (inst.name == "getelementptr inbounds") {
        getelementptr(inst);
    }
    else if (inst.name == "call") {
        if (inst.ops[0].reg == "@putint") putint(inst);
        else if (inst.ops[0].reg == "@putstr") putstr(inst);
        else if (inst.ops.size() >= 2 && inst.ops[1].reg == "@getint") getint(inst);
        else {
            save();
            int space = call(inst);
            restore(space, inst);
        }
    }
    else if (inst.name == "ret") {
        pool.clean();
        int num = -table["$sp"];
        MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(num));
        text.add(addiu);
        ret(inst);
    }
    else if (inst.name == "br") {
        pool.clean();
        br(inst);
    }
    else if (inst.name == "label") {
        MInst label("label", 1, Op(inst.ops[0].reg));
        text.add(label);
    }
    else if (inst.name == "icmp") {
        icmp(inst);
    }
    else if (inst.name == "zext") {
        pool.replace(inst.ops[1].reg, inst.ops[0].reg);
    }
}

void transBlk(Blk& blk) {
    for (int i = 0; i < blk.insts.size(); i++) {
        transInst(blk.insts[i]);
    }
}

void transFunc(Function& f) {
    if (f.ret.reg == "@getint" || f.ret.reg == "@putint" || f.ret.reg == "@putstr") return;
    table.clear();
    table.insert({"$sp", 0});

    for (int i = 0; i < f.args.size(); i++) {
        table.insert({f.args[i].reg, 4*i});
    }

    string label = string(f.ret.reg.begin()+1, f.ret.reg.end());
    MInst m("label", 1, Op(label));
    text.add(m); //function name label

    for (int i = 0; i < f.blks.size(); i++) {
        transBlk(f.blks[i]);
    }
}

void transIR(IR& ir) {
    transBlk(ir.global);
    MInst jal("jal", 1, Op("main"));
    text.add(jal);
    MInst nop("nop", 0);
    text.add(nop);

    for (int i = 0; i < ir.functions.size(); i++) {
        isMain = ir.functions[i].ret.reg == "@main";
        transFunc(ir.functions[i]);
    }
}

void translate(IR& ir) {
    transIR(ir);
    mips.data = data;
    mips.text = text;
    FILE* fp = fopen("mips.txt", "w");
    mips.print(fp);
    fclose(fp);
}

}