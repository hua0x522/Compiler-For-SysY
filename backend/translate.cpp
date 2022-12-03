#include "translate.h"

namespace TRANS
{

MIPS mips;
Segment data;
Segment text;
vector<string> saved;
map<string, int> table;
bool isMain;
Function* f;
Blk* BB;
int bb_idx;
int inst_idx;
 Pool pool(24, "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9", "$a1", "$a2", "$a3",
               "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$v1", "$k0", "$k1");
//Pool pool(4, "$t0", "$t1", "$t2", "$t3");

string Pool::alloc(string vreg) {
    counter++;
    for (int i = 0; i < regs.size(); i++) {
        if (regs[i].use == "") {
            regs[i].use = vreg;
            ages[i] = counter;
            return regs[i].id;
        }
    }
    int idx = 0;
    for (int i = 0; i < regs.size(); i++) {
        if (ages[i] < ages[idx]) {
            idx = i;
        }
    }
    ages[idx] = counter;
    vector<int> vis(f->blks.size(), 0);
    if (useful(regs[idx].use, bb_idx, inst_idx, vis)) {
        int off = table[regs[idx].use] - table["$sp"];
        MInst sw("sw", 3, Op(regs[idx].id), Op(off), Op("$sp"));
        text.add(sw);
    }
    regs[idx].use = vreg;
    return regs[idx].id;
}

void Pool::clean() {
//    inst_idx -= 1;
    for (int i = 0; i < regs.size(); i++) {
        vector<int> vis(f->blks.size(), 0);
        if (regs[i].use != "" && useful(regs[i].use, bb_idx, inst_idx, vis)) {
            int off = table[regs[i].use] - table["$sp"];
            MInst sw("sw", 3, Op(regs[i].id), Op(off), Op("$sp"));
            text.add(sw);
        }
    }
//    inst_idx += 1;
}

bool useful(string vreg, int bbIdx, int instIdx, vector<int>& vis) {
    /*
     * bbIdx: block idx in function, instIdx: instruction idx in block
     */
    auto blk = f->blks.begin() + bbIdx;
    for (auto it = blk->insts.begin() + instIdx; it != blk->insts.end(); it++) {
        string res = it->getRes();
        for (auto op = it->ops.begin(); op != it->ops.end(); op++) {
            if (op->toString() == res && vreg == op->toString()) return false;
            if (vreg == op->toString()) return true;
        }
        if (it->name == "br") {
            if (it->ops.size() == 1) {
                int idx = f->getBlk(it->ops[0].toString());
                if (vis[idx] == 0) {
                    vis[idx] = 1;
                    if (useful(vreg, idx, 0, vis)) return true;
                }
            }
            else {
                int idx1 = f->getBlk(it->ops[1].toString());
                int idx2 = f->getBlk(it->ops[2].toString());
                if (vis[idx1] == 0) {
                    vis[idx1] = 1;
                    if (useful(vreg, idx1, 0, vis)) return true;
                }
                if (vis[idx2] == 0) {
                    vis[idx2] = 1;
                    if (useful(vreg, idx2, 0, vis)) return true;
                }
            }
        }
    }
    return false;
}

string val2reg(Value v, bool isDef) {
    string reg;
    if (v.reg == "") {
        reg = "$a0";
        MInst li("li", 2, Op(reg), Op(v.num));
        text.add(li);
    }
    else if (v.reg[0] == '@') {
        return getAddr(v);
    }
    else {
        reg = pool.query(v.reg);
        if (reg == "") {
            reg = pool.alloc(v.reg);
            if (!isDef) {
                int off = table[v.reg] - table["$sp"];
                MInst lw("lw", 3, Op(reg), Op(off), Op("$sp"));
                text.add(lw);
            }
        }
    }
    return reg;
}

string getAddr(Value v) {
    string addr = string(v.reg.begin()+1, v.reg.end());
    MInst la("la", 2, Op("$gp"), Op(addr));
    text.add(la);
    return "$gp";
}

void printTB() {
    for (auto it = table.begin(); it != table.end(); it++) {
        printf("%s %d, ", it->first.c_str(), it->second);
    }
    printf("\n");
}

void global_data(Inst& inst) {
    string label = string(inst.ops[0].reg.begin()+1, inst.ops[0].reg.end());
    if (inst.ops[0].ty.data == "i8") {
        MInst m(".asciiz", 2, Op(label), Op(inst.ops[1].reg));
        data.add(m);
        return;
    }
    int size = (inst.ops[0].ty.shape.size() == 0) ? 1 : inst.ops[0].ty.shape[0];
    MInst m(".word", 1, Op(label));
    for (int i = 0; i < size; i++) {
        if (i+1 < inst.ops.size()) {
           m.ops.push_back(inst.ops[i+1].num);
        }
        else {
            m.ops.push_back(0);
        }
    }
    data.add(m);
}

void local_data(Inst& inst) {
    /*   if size != 0, op must be not a ptr (we only have 1dim arr)  */
    int size = (inst.ops[0].ty.shape.size() == 0) ? 1 : inst.ops[0].ty.shape[0];    
    MInst m("addiu", 3, Op("$sp"), Op("$sp"), Op(-4*size));
    text.add(m);
    table["$sp"]  -= 4 * size;
    string addr = val2reg(inst.ops[0], true);
    MInst move("move", 2, Op(addr), Op("$sp"));
    text.add(move);
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
        string reg1 = val2reg(inst.ops[1], false);
        string reg2 = val2reg(inst.ops[2], false);
        string name = (inst.name == "add") ? "addu" :
                      (inst.name == "sub") ? "subu" :
                      (inst.name == "mul") ? "mul"   :
                      (inst.name == "sdiv")? "div"   :
                      (inst.name == "srem")? "div"   : 0;
        MInst m(name, 3, Op(res), Op(reg1), Op(reg2));
        text.add(m);
        if (inst.name == "srem") {
            MInst mfhi("mfhi", 1, Op(res));
            text.add(mfhi);
        }
    }
}

void load(Inst& inst) {
    string res = val2reg(inst.ops[0], true);
    if (inst.ops[1].reg[0] == '@') {
        string addr = string(inst.ops[1].reg.begin()+1, inst.ops[1].reg.end());
        MInst lw("lw", 2, Op(res), Op(addr));
        text.add(lw);
    }
    else {
        string addr = val2reg(inst.ops[1], false);
        MInst lw("lw", 2, Op(res), Op(addr));
        text.add(lw);
    }
}

void store(Inst& inst) {
    string val = val2reg(inst.ops[0], false);
    if (inst.ops[1].reg[0] == '@') {
        string addr = string(inst.ops[1].reg.begin()+1, inst.ops[1].reg.end());
        MInst sw("sw", 2, Op(val), Op(addr));
        text.add(sw);
    }
    else {
        string addr = val2reg(inst.ops[1], false);
        MInst sw("sw", 2, Op(val), Op(addr));
        text.add(sw);
    }
}

void icmp(Inst& inst) {
    map<string, string> imap = {
        {"eq", "seq"}, {"ne", "sne"}, {"sgt", "sgt"}, 
        {"sge", "sge"}, {"slt", "slt"}, {"sle", "sle"}
    };
    string res = pool.alloc(inst.ops[0].reg);
    string tp = inst.ops[1].reg;

    if (inst.ops[2].reg == "" && inst.ops[3].reg == "") {
        int v1 = inst.ops[2].num, v2 = inst.ops[3].num;
        int num = (imap[tp] == "seq") ? v1 == v2 :
                  (imap[tp] == "sne") ? v1 != v2 :
                  (imap[tp] == "sgt") ? v1 > v2 :
                  (imap[tp] == "sge") ? v1 >= v2 :
                  (imap[tp] == "slt") ? v1 < v2 :
                  (imap[tp] == "sle") ? v1 <= v2 : 0;
        MInst li("li", 2, Op(res), Op(num));
        text.add(li);
    }
    else {
        string v1 = val2reg(inst.ops[2], false);
        string v2 = val2reg(inst.ops[3], false);
        MInst m(imap[tp], 3, Op(res), Op(v1), Op(v2));
        text.add(m);
    }
}

void br(Inst& inst) {
    if (inst.ops.size() == 1) {
        MInst j("j", 1, Op(inst.ops[0].reg));
        text.add(j);
    }
    else {
        string v = val2reg(inst.ops[0], false);
        string l1 = inst.ops[1].reg;
        string l2 = inst.ops[2].reg;
        MInst bgtz("bgtz", 2, Op(v), Op(l1));
        text.add(bgtz);
        MInst beqz("beqz", 2, Op(v), Op(l2));
        text.add(beqz);
    }
}

void getelementptr(Inst& inst) {    
    string res = val2reg(inst.ops[0], true);
    string addr = val2reg(inst.ops[1], false);
    int idx = inst.ops.size()-1;
    if (inst.ops[idx].reg == "") {
        MInst addiu("addiu", 3, Op(res), Op(addr), Op(4*inst.ops[idx].num));
        text.add(addiu);
    }
    else {
        string reg = pool.query(inst.ops[idx].reg);
        MInst sll("sll", 3, Op(reg), Op(reg), Op(2));
        text.add(sll);
        MInst addu("addu", 3, Op(res), Op(addr), Op(reg));
        text.add(addu);
    }
}

void save() {
    for (auto it = pool.regs.begin(); it != pool.regs.end(); it++) {
        vector<int> vis(f->blks.size(), 0);
        if ((*it).use != "" && useful((*it).use, bb_idx, inst_idx, vis)) {
            saved.push_back((*it).id);
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
//            string reg = (inst.ops[i].ty.ptr) ? getAddr(inst.ops[i]) : val2reg(inst.ops[i], false);
            string reg = val2reg(inst.ops[i], false);
            MInst sw("sw", 3, Op(reg), Op(4*cnt), Op("$sp")); 
            text.add(sw);
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
    for (int i = 0; i < pool.regs.size(); i++) {
        bool inSaved = false;
        for (int j = 0; j < saved.size(); j++) {
            if (saved[j] == pool.regs[i].id) inSaved = true;
        }
        if (!inSaved) pool.regs[i].use = "";
    }
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
    int num = -table["$sp"];
    if (isMain) {
        MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(num));
        text.add(addiu);
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
            string reg = val2reg(inst.ops[0], false);
            MInst move("move", 2, Op("$v0"), Op(reg));
            text.add(move);
        }
    }
    MInst addiu("addiu", 3, Op("$sp"), Op("$sp"), Op(num));
    text.add(addiu);
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
    string reg = val2reg(inst.ops[1], false);
    MInst move("move", 2, Op("$a0"), Op(reg));
    text.add(move);
    MInst li("li", 2, Op("$v0"), Op(1));
    text.add(li);
    MInst syscall("syscall", 0);
    text.add(syscall);
}

void putstr(Inst& inst) {
    string reg = val2reg(inst.ops[1], false);
    MInst move("move", 2, Op("$a0"), Op(reg));
    text.add(move);
    MInst li("li", 2, Op("$v0"), Op(4));
    text.add(li);
    MInst syscall("syscall", 0);
    text.add(syscall);
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
//        pool.clean();
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
        string reg = val2reg(inst.ops[1], false);
        pool.replace(inst.ops[1].reg, inst.ops[0].reg);
    }
    else if (inst.name == "move") {
        if (inst.ops[0].reg == "") {
            string reg = val2reg(inst.ops[1], true);
            MInst li("li", 2, Op(reg), Op(inst.ops[0].num));
            text.add(li);
        }
        else {
            string reg0 = val2reg(inst.ops[0], false);
            string reg1 = val2reg(inst.ops[1], true);
            MInst mv("move", 2, Op(reg1), Op(reg0));
            text.add(mv);
        }
    }
}

void transBlk(Blk& blk) {
    for (int i = 0; i < blk.insts.size(); i++) {
        inst_idx = i;
        transInst(blk.insts[i]);
    }
    pool.reset();
}

void allocOverflow(Function& func) {
    set<string> vregs;
    for (auto op = func.args.begin(); op != func.args.end(); op++) {
        vregs.insert(op->toString());
        table[op->toString()] = 4 * (op - func.args.begin());
    }
    for (auto bb = func.blks.begin(); bb != func.blks.end(); bb++) {
        for (auto it = bb->insts.begin(); it != bb->insts.end(); it++) {
            for (auto val = it->ops.begin(); val != it->ops.end(); val++) {
                if (val->reg != "" && val->reg[0] == '%') {
                    if (vregs.find(val->reg) == vregs.end()) {
                        table["$sp"] -= 4;
                        table[val->reg] = table["$sp"];
                        vregs.insert(val->toString());
                    }
                }
            }
        }
    }
    MInst alloc("addiu", 3, Op("$sp"), Op("$sp"), Op(-4 * (vregs.size() - f->args.size()) ));
    text.add(alloc);
}

void transFunc(Function& func) {
    if (func.ret.reg == "@getint" || func.ret.reg == "@putint" || func.ret.reg == "@putstr") return;
    f = &func;
    table.clear();
    table.insert({"$sp", 0});

    for (int i = 0; i < func.args.size(); i++) {
        table.insert({func.args[i].reg, 4*i});
    }

    string label = string(func.ret.reg.begin()+1, func.ret.reg.end());
    MInst m("label", 1, Op(label));
    text.add(m); //function name label
    allocOverflow(func); // alloc stack space for overflow

    for (int i = 0; i < func.blks.size(); i++) {
        bb_idx = i;
        transBlk(func.blks[i]);
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