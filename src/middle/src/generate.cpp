#include "generate.h"

namespace GEN
{
Table table;
vector<int> cbuf;
vector<Value> vbuf;
int blocks;
IR ir;
Function function;
Blk global;
string funcType;
Type i32("i32", vector<int>(), 0);
Type i32p("i32", vector<int>(), 1);
Type i1("i1", vector<int>(), 0);
Type i8("i8", vector<int>(), 0);
Type i8p("i8", vector<int>(), 1);
vector<string> while_begin;
vector<string> while_end;

string newReg() {
    static int regs = 0;
    regs++;
    string reg("\%x");
    reg += i2s(regs);
    return reg;
}

string newLabel() {
    static int l = 0;
    l++;
    string label("br_");
    label += i2s(l);
    if (global.getVal("@"+label).reg != "") return newLabel();
    else return label;
}

string newStr() {
    static int cnt = 0;
    cnt++;
    string s("@str_");
    s += i2s(cnt);
    if (global.getVal(s).reg != "") return newStr();
    else return s;
}

void i1toi32(Value& v) {
    Value res(newReg(), i32);
    Inst inst("zext", 2, res, v);
    function.add(inst);
    v = res;
}

void i32toi1(Value& v) {
    Value res(newReg(), i1);
    Inst inst("icmp", 4, res, Value("ne", i32), v, Value(0, i32));
    function.add(inst);
    v = res;
}

void declare() {
    Function f;
    Value ret("@getint", i32);
    f.ret = ret;
    ir.add(f);
    f.ret.reg = "@putint";
    f.ret.ty.data = "void";
    f.args.push_back(Value("", i32));
    ir.add(f);
    f.ret.reg = "@putstr";
    f.args.clear();
    f.args.push_back(Value("", i8p));
    ir.add(f);
}

void gCompUnit(Node* node) {
    declare();
    auto it = node->sons.begin();
    while((*it)->token.val == "<Decl>") {
        gDecl(*it);
        it++;
    }
    ir.global = global;
    function.clear();
    while ((*it)->token.val == "<FuncDef>") {
        gFuncDef(*it);
        it++;
        function.checkRet();
        function.divBlk();
        ir.add(function);
        function.clear();
    }
    gMainFuncDef(*it);
    function.divBlk();
    ir.add(function);
    function.clear();
    ir.global = global; //update constant string 
}

void gDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstDecl>") gConstDecl(*it);
        else if ((*it)->token.val == "<VarDecl>") gVarDecl(*it);
    }
}

void gConstDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstDef>") gConstDef(*it);
    }
}

void gConstDef(Node* node) {
    Var var("", "const int", 0);
    var.pos = cbuf.size();
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") var.id = (*it)->token.val;
        else if ((*it)->token.val == "[") var.dim += 1;
        else if ((*it)->token.val == "<ConstExp>") var.dims.push_back(gConstExp(*it));
        else if ((*it)->token.val == "<ConstInitVal>") gConstInitVal(*it);
    }
    var.reg = (blocks == 0) ? "@" + var.id : newReg();
    table.add(var);
    Type ty("i32", var.flat(), 1);
    Value val(var.reg, ty);
    string name = (blocks) ? "alloca" : "constant";
    Inst inst(name, 1, val);
    if (inst.name == "alloca") function.add(inst);
    int cnt = (var.flat().size()) ? var.flat()[0] : 1;
    if (inst.name == "constant")  {
        for (int i = var.pos; i < var.pos + cnt; i++) {
            inst.ops.push_back(Value(cbuf[i], i32));
        }
        global.add(inst);
        return;
    }
    if (val.ty.shape.size() == 0) {
        Value v(cbuf[var.pos], i32);
        Inst inst("store", 2, v, val);
        function.add(inst);
    }
    else {
        for (int i = 0; i < cnt; i++) {
            Value addr(newReg(), i32p);
            Inst inst1("getelementptr inbounds", 4, addr, val, Value(0, i32), Value(i, i32));
            function.add(inst1);
            Value v(cbuf[var.pos+i], i32);
            Inst inst2("store", 2, v, addr);
            function.add(inst2);
        }
    } 
}

void gConstInitVal(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<ConstExp>") cbuf.push_back(gConstExp(*it));
        else if ((*it)->token.val == "<ConstInitVal>") gConstInitVal(*it);
    } 
}

void gVarDecl(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<VarDef>") gVarDef(*it);
    }
}

void gVarDef(Node* node) {
    Var var("", "int", 0);
    if (!blocks) var.pos = cbuf.size();
    else var.pos = vbuf.size();
    auto it = node->sons.begin();
    int flag = 0;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") var.id = (*it)->token.val;
        else if ((*it)->token.val == "[") var.dim += 1;
        else if ((*it)->token.val == "<ConstExp>") var.dims.push_back(gConstExp(*it));
        else if ((*it)->token.val == "<InitVal>") flag = 1;
    }
    var.reg = (blocks == 0) ? "@" + var.id : newReg();
    table.add(var);
    Type ty("i32", var.flat(), 1);
    Value val(var.reg, ty);
    string name = (blocks) ? "alloca" : "global";
    Inst inst(name, 1, val);
    if (!flag) {
        if (inst.name == "global") global.add(inst);
        else function.add(inst);
        return;
    }
    if (inst.name == "alloca") function.add(inst);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<InitVal>") gInitVal(*it);
    }
    int cnt = (var.flat().size()) ? var.flat()[0] : 1;
    if (inst.name == "global")  {
        for (int i = var.pos; i < var.pos + cnt; i++) {
            inst.ops.push_back(Value(cbuf[i], i32));
        }
        global.add(inst);
        return;
    }
    if (val.ty.shape.size() == 0) {
        Value v = vbuf[var.pos];
        Inst inst("store", 2, v, val);
        function.add(inst);
    }
    else {
        for (int i = 0; i < cnt; i++) {
            Value addr(newReg(), i32p);
            Inst inst1("getelementptr inbounds", 4, addr, val, Value(0, i32), Value(i, i32));
            function.add(inst1);
            Value v = vbuf[var.pos+i];
            Inst inst2("store", 2, v, addr);
            function.add(inst2);
        }
    } 
}

void gInitVal(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") {
            if (!blocks) cbuf.push_back(gConstExp(*it));
            else vbuf.push_back(gExp(*it));
        }
        else if ((*it)->token.val == "<InitVal>") gInitVal(*it);
    }
}

void gFuncDef(Node* node) {
    Value ret;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<FuncType>") {
            gFuncType(*it);
            Type ty(funcType, vector<int>(), 0);
            ret.ty = ty;
        }
        else if ((*it)->token.tp == "IDENFR") {
            ret.reg = "@" + (*it)->token.val;
            function.ret = ret;
            table.inScope();
        }
        else if ((*it)->token.val == "<FuncFParams>") gFuncFParams(*it);
        else if ((*it)->token.val == "<Block>") {
            gBlock(*it);
            table.outScope();
        }
    }
}

void gMainFuncDef(Node* node) {
    Value ret("@main", i32);
    function.ret = ret;
    table.inScope();
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Block>") {
            gBlock(*it);
            table.outScope();
        }
    }
}

void gFuncType(Node* node) {
    auto it = node->sons.begin();
    if ((*it)->token.val == "void") funcType = "void";
    else if ((*it)->token.val == "int") funcType = "i32";
}

void gFuncFParams(Node* node) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<FuncFParam>") gFuncFParam(*it);
    }
}

void gFuncFParam(Node* node) {
    Type ty = i32;
    Value val(newReg(), i32);
    Var var("","int",0);
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.tp == "IDENFR") var.id = (*it)->token.val;
        else if ((*it)->token.val == "[" && (*(it+1))->token.val == "]") {
            ty.ptr = 1;
            var.dims.push_back(1);
        }
        else if ((*it)->token.val == "<ConstExp>") var.dims.push_back(gConstExp(*it));
    }
    val.ty = ty;
    function.args.push_back(val);
    ty.ptr++;
    Value addr(newReg(), ty);
    Inst inst1("alloca", 1, addr);
    function.add(inst1);
    Inst inst2("store", 2, val, addr);
    function.add(inst2);
    var.reg = addr.reg;
    table.add(var);
}

void gBlock(Node* node) {
    if (blocks) table.inScope();
    blocks++;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<BlockItem>") gBlockItem(*it);
    }
    blocks--;
    if (blocks) table.outScope();
}

void gBlockItem(Node* node) {
    auto it = node->sons.begin();
    if (node->sons[0]->token.val == "<Decl>") gDecl(*it);
    else if (node->sons[0]->token.val == "<Stmt>") gStmt(*it);
}

void gStmt(Node* node) {
    auto it = node->sons.begin();
    if ((*it)->token.val == "<LVal>") {
        Value lval = gLVal(*it, true);
        if ((*(it+2))->token.val == "<Exp>") {
            Value exp = gExp(*(it+2));
            Inst inst("store", 2, exp, lval);
            function.add(inst);
        }
        else if ((*(it+2))->token.val == "getint") {
            Value v(newReg(), i32);
            Function f = ir.getFunc("@getint");
            Inst call("call", 2, v, f.ret);
            function.add(call);
            Inst store("store", 2, v, lval);
            function.add(store);
        }
    }
    else if ((*it)->token.val == "<Exp>") gExp(*it);
    else if ((*it)->token.val == "<Block>") gBlock(*it);
    else if ((*it)->token.val == "return") {
        if ((*(it+1))->token.val == "<Exp>") {
            Value v = gExp(*(it+1));
            Inst inst("ret", 1, v);
            function.add(inst);
        }
        else {
            Type ty("void", vector<int>(), 0);
            Value v(0, ty);
            Inst inst("ret", 1, v);
            function.add(inst);
        }
    }
    else if ((*it)->token.val == "printf") {
        string s;
        vector<Value> vals;
        for (auto i = node->sons.begin(); i != node->sons.end(); i++) {
            if ((*i)->token.tp == "STRCON") s = (*i)->token.val;
            else if ((*i)->token.val == "<Exp>") vals.push_back(gExp(*i));
        }
        Function pi = ir.getFunc("@putint");
        string temp;
        for (int i = 1; i < s.size()-1; i++) {
            if (s[i] == '%') {
                putstr(temp);
                Inst inst("call", 2, pi.ret, vals[0]);
                function.add(inst);
                vals.erase(vals.begin());
                i++;
            }
            else if (s[i] == '\\') {
                temp += string(1, '\n');
                i++;
            }
            else {
                temp += string(1, s[i]);
            }
        }
        putstr(temp);
    }
    else if ((*it)->token.val == "if") {
        string l1 = newLabel();
        string l2 = newLabel();
        gCond(*(it+2), l1, l2);
        if (node->sons.size() > 5) {
            string end = newLabel();
            Inst label1("label", 1, Value(l1, i32));
            function.add(label1); 
            gStmt(*(it+4));
            Inst br("br", 1, Value(end, i32));
            function.add(br);
            Inst label2("label", 1, Value(l2, i32));
            function.add(label2);
            gStmt(*(it+6));
            Inst l_end("label", 1, Value(end, i32));
            function.add(l_end);
        }
        else {
            Inst label1("label", 1, Value(l1, i32));
            function.add(label1); 
            gStmt(*(it+4));
            Inst br("br", 1, Value(l2, i32));
            function.add(br);
            Inst label2("label", 1, Value(l2, i32));
            function.add(label2);
        }
    }
    else if ((*it)->token.val == "while") {
        string begin = newLabel();
        string l1 = newLabel();
        string end = newLabel();
        while_begin.push_back(begin);
        while_end.push_back(end);
        Inst l_begin("label", 1, Value(begin, i32));
        function.add(l_begin);
        gCond(*(it+2), l1, end);
        Inst l_l1("label", 1, Value(l1, i32));
        function.add(l_l1);
        gStmt(*(it+4));
        Inst br("br", 1, Value(begin, i32));
        function.add(br);
        Inst l_end("label", 1, Value(end, i32));
        function.add(l_end);
        while_begin.pop_back();
        while_end.pop_back();
    }
    else if ((*it)->token.val == "break") {
        Inst inst("br", 1, Value(*(while_end.end()-1), i32));
        function.add(inst);
    }
    else if ((*it)->token.val == "continue") {
        Inst inst("br", 1, Value(*(while_begin.end()-1), i32));
        function.add(inst);
    }
}

void putstr(string& temp) {
    if (!temp.size()) return;
    string s = newStr();
    Value str(s, i8p);
    str.ty.shape.push_back(temp.size()+1);
    Inst constant("constant", 2, str, Value(temp, i32));
    global.add(constant);
    Value res(newReg(), i8p);
    Inst gep("getelementptr inbounds", 4, res, str, Value(0, i32), Value(0, i32));
    function.add(gep);
    Function ps = ir.getFunc("@putstr");
    Inst call("call", 2, ps.ret, res);
    function.add(call);
    temp = "";
}

Value gExp(Node* node) {
    return gAddExp(node->sons[0]);
}

Value gLVal(Node* node, bool isAddr) {
    Var var = table.findVar(node->sons[0]->token.val);
    Value val = function.getVal(var.reg);
    if (val.reg == "") val = ir.getVal(var.reg);
    vector<Value> ind;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") {
            Value v = gExp(*it);
            ind.push_back(v);
        }
    }
    if (!ind.size()) {
        if (isAddr) return val;
        Type ty = val.ty;
        Value res(newReg(), i32);
        if (!ty.shape.size()) {
            ty.ptr--;
            res.ty = ty;
            Inst inst("load", 2, res, val);
            function.add(inst);
        }
        else {
            ty.shape.pop_back();
            res.ty = ty;
            Inst inst("getelementptr inbounds", 4, res, val, Value(0, i32), Value(0, i32));
            function.add(inst);
        }
        return res;
    }
    if (val.ty.ptr == 2) {
        Type ty = val.ty;
        ty.ptr--;
        Value res(newReg(), ty);
        Inst inst("load", 2, res, val);
        function.add(inst);
        val = res;
    }
    int num = var.flat()[0];
    Type ty = i32;
    Value v(num, ty);
    for (int i = 0; i < ind.size(); i++) {
        v.num /= var.dims[i];
        Value res(newReg(), ty);
        string name = (i == 0) ? "mul" : "add";   /*   array dim leq 2   */
        Inst inst(name, 3, res, ind[i], v);
        function.add(inst);
        v = res;
    }
    ty = i32p;
    Value addr(newReg(), ty);
    Inst inst("getelementptr inbounds", 2, addr, val);
    if (val.ty.shape.size() != 0) {         //is array
        inst.ops.push_back(Value(0, i32));  /*     array is different from pointer     */
    }
    inst.ops.push_back(v);
    function.add(inst);
    if (ind.size() < var.dims.size()) isAddr = true;
    if (isAddr) return addr; 
    ty.ptr--;
    Value res(newReg(), ty);
    Inst inst2("load", 2, res, addr);
    function.add(inst2);
    return res;
}

Value gPrimaryExp(Node* node) {
    auto it = node->sons.begin();
    if ((*it)->token.val == "(") {
        return gExp(*(it+1));
    }
    else if ((*it)->token.val == "<LVal>") return gLVal(*it, false);
    else if ((*it)->token.val == "<Number>") return gNumber(*it);
    return Value(0, i32);
}

Value gNumber(Node* node) {
    string s = node->sons[0]->token.val;
    int num = 0;
    for (int i = 0; i < s.size(); i++) {
        num = num * 10 + s[i] - '0';
    }
    Value v(num, i32);
    return v;
}

Value gUnaryExp(Node* node) {
    auto it = node->sons.begin();
    if ((*it)->token.val == "<PrimaryExp>") {
        return gPrimaryExp(*it);
    }
    else if ((*it)->token.tp == "IDENFR") {
        Function f = ir.getFunc("@" + (*it)->token.val);
        if (f.ret.reg == "") f = function;
        Inst inst("call", 0);
        if (f.ret.ty.data == "i32") {
            Value res(newReg(), f.ret.ty);
            inst.ops.push_back(res);
        }
        inst.ops.push_back(f.ret);
        if ((*(it+2))->token.val == "<FuncRParams>") {
            vector<Value> vals = gFuncRParams(*(it+2));
            for (int i = 0; i < vals.size(); i++) {
                inst.ops.push_back(vals[i]);
            }
        } 
        function.add(inst);
        if (inst.ops[0].reg != f.ret.reg) return inst.ops[0];
        /*     only return i32     */
    }
    else if ((*it)->token.val == "<UnaryOp>") {
        Value v2 = gUnaryExp(*(it+1));
        if ((*it)->sons[0]->token.val == "-") {
            Value v1(0, i32);
            Value res(newReg(), i32);
            if (v2.ty.like(i1)) i1toi32(v2);
            Inst inst("sub", 3, res, v1, v2);
            function.add(inst);
            v2 = res;
        } 
        else if ((*it)->sons[0]->token.val == "!") {
            if (v2.ty.like(i1)) i1toi32(v2);
            Value res(newReg(), i1);
            Inst inst("icmp", 4, res, Value("eq", i32), v2, Value(0, i32));
            function.add(inst);
            v2 = res;
        }
        return v2;
    }
    return Value(0, i32); // void function return 0
}

vector<Value> gFuncRParams(Node* node) {
    vector<Value> vals;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Exp>") vals.push_back(gExp(*it));
    }
    return vals;
}

Value gMulExp(Node* node) {
    Value v1 = gUnaryExp(node->sons[0]);
    string name;
    for (auto it = node->sons.begin()+1; it != node->sons.end(); it++) {
        if ((*it)->token.val == "*") name = "mul";
        else if ((*it)->token.val == "/") name = "sdiv";
        else if ((*it)->token.val == "%") name = "srem";
        else if ((*it)->token.val == "<UnaryExp>") {
            Value v2 = gUnaryExp(*it);
            Value res(newReg(), i32);
            if (v1.ty.like(i1)) i1toi32(v1);
            if (v2.ty.like(i1)) i1toi32(v2);
            Inst inst(name, 3, res, v1, v2);
            function.add(inst);
            v1 = res;
        }
    }
    return v1;
}

Value gAddExp(Node* node) {
    Value v1 = gMulExp(node->sons[0]);
    string name;
    for (auto it = node->sons.begin()+1; it != node->sons.end(); it++) {
        if ((*it)->token.val == "+") name = "add";
        else if ((*it)->token.val == "-") name = "sub";
        else if ((*it)->token.val == "<MulExp>") {
            Value v2 = gMulExp(*it);
            Value res(newReg(), i32);
            if (v1.ty.like(i1)) i1toi32(v1);
            if (v2.ty.like(i1)) i1toi32(v2);
            Inst inst(name, 3, res, v1, v2);
            function.add(inst);
            v1 = res;
        }
    }
    return v1;
}

void gCond(Node* node, string l1, string l2) {
    auto it = node->sons.begin();
    gLOrExp(*it, l1, l2); 
}

void gLOrExp(Node* node, string l1, string l2) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<LAndExp>") {
            if (it+1 != node->sons.end()) {
                string l = newLabel();
                gLAndExp(*it, l1, l);
                Inst label("label", 1, Value(l, i32));
                function.add(label);
            }
            else {
                gLAndExp(*it, l1, l2);
            }
        }
    }
}

void gLAndExp(Node* node, string l1, string l2) {
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<EqExp>") {
            if (it+1 != node->sons.end()) {
                string l = newLabel();
                Value v = gEqExp(*it);
                Inst inst("br", 3, v, Value(l, i32), Value(l2, i32));
                function.add(inst);
                Inst label("label", 1, Value(l, i32));
                function.add(label);
            }
            else {
                Value v = gEqExp(*it);
                Inst inst("br", 3, v, Value(l1, i32), Value(l2, i32));
                function.add(inst);
            }
        }
    }
}

Value gEqExp(Node* node) {
    auto it = node->sons.begin();
    Value v0 = gRelExp(*it);
    it++;
    while (it != node->sons.end()) {
        string name;
        if ((*it)->token.val == "==") name = "eq";
        else name = "ne";
        it++;
        Value v = gRelExp(*it);
        if (v0.ty.like(i1)) i1toi32(v0); 
        if (v.ty.like(i1)) i1toi32(v);
        Value res(newReg(), i1);
        Inst inst("icmp", 4, res, Value(name, i32), v0, v);
        function.add(inst);
        v0 = res;
        it++;
    }
    if (v0.ty.like(i32)) i32toi1(v0);
    return v0;
}

Value gRelExp(Node* node) {
    auto it = node->sons.begin();
    Value v0 = gAddExp(*it);
    it++;
    while (it != node->sons.end()) {
        string name;
        if ((*it)->token.val == ">=") name = "sge";
        else if ((*it)->token.val == ">") name = "sgt";
        else if ((*it)->token.val == "<=") name = "sle";
        else name = "slt";
        it++;
        Value v = gAddExp(*it);
        if (v0.ty.like(i1)) i1toi32(v0); 
        if (v.ty.like(i1)) i1toi32(v);
        Value res(newReg(), i1);
        Inst inst("icmp", 4, res, Value(name, i32), v0, v);
        function.add(inst);
        v0 = res;
        it++;
    }
    return v0;
}

IR generate(Node* root) {
    gCompUnit(root);
    FILE* fp = fopen("llvm_ir.txt", "w");
    printIR(fp);
    fclose(fp);
    return ir;
}

}