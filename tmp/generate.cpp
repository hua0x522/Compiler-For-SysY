#include "generate.h"

namespace GEN
{
Table tab;
vector<int> cbuf;
vector<Value> vbuf;
int blocks;
IR ir;
Function function;
string funcType;
Type i32("i32", vector<int>(), 0);
Type i32p("i32", vector<int>(), 1);

string newReg() {
    static int regs = 0;
    regs++;
    string reg("\%x");
    reg += i2s(regs);
    return reg;
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
    f.ret.reg = "@putch";
    ir.add(f);
}

void gCompUnit(Node* node) {
    declare();
    auto it = node->sons.begin();
    while((*it)->token.val == "<Decl>") {
        gDecl(*it);
        it++;
    }
    ir.add(function);
    function.clear();
    while ((*it)->token.val == "<FuncDef>") {
        gFuncDef(*it);
        it++;
        function.checkRet();
        ir.add(function);
        function.clear();
    }
    gMainFuncDef(*it);
    ir.add(function);
    function.clear();
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
    tab.add(var);
    Type ty("i32", var.flat(), 1);
    Value val(var.reg, ty);
    Inst inst(1, val);
    inst.name = (blocks) ? "alloca" : "constant";
    if (inst.name == "alloca") function.add(inst);
    int cnt = (var.flat().size()) ? var.flat()[0] : 1;
    if (inst.name == "constant")  {
        for (int i = var.pos; i < var.pos + cnt; i++) {
            inst.ops.push_back(Value(cbuf[i], i32));
        }
        function.add(inst);
        return;
    }
    if (val.ty.shape.size() == 0) {
        Value v(cbuf[var.pos], i32);
        Inst inst(2, v, val);
        inst.name = "store";
        function.add(inst);
    }
    else {
        for (int i = 0; i < cnt; i++) {
            Value addr(newReg(), i32p);
            Inst inst1(4, addr, val, Value(0, i32), Value(i, i32));
            inst1.name = "getelementptr inbounds";
            function.add(inst1);
            Value v(cbuf[var.pos+i], i32);
            Inst inst2(2, v, addr);
            inst2.name = "store";
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
    tab.add(var);
    Type ty("i32", var.flat(), 1);
    Value val(var.reg, ty);
    Inst inst(1, val);
    inst.name = (blocks) ? "alloca" : "global";
    if (!flag) {
        if (inst.name == "global") inst.ops.push_back(Value(0, i32));
        function.add(inst);
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
        function.add(inst);
        return;
    }
    if (val.ty.shape.size() == 0) {
        Value v = vbuf[var.pos];
        Inst inst(2, v, val);
        inst.name = "store";
        function.add(inst);
    }
    else {
        for (int i = 0; i < cnt; i++) {
            Value addr(newReg(), i32p);
            Inst inst1(4, addr, val, Value(0, i32), Value(i, i32));
            inst1.name = "getelementptr inbounds";
            function.add(inst1);
            Value v = vbuf[var.pos+i];
            Inst inst2(2, v, addr);
            inst2.name = "store";
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
            tab.inScope();
        }
        else if ((*it)->token.val == "<FuncFParams>") gFuncFParams(*it);
        else if ((*it)->token.val == "<Block>") {
            gBlock(*it);
            tab.outScope();
        }
    }
    function.ret = ret;
}

void gMainFuncDef(Node* node) {
    Value ret("@main", i32);
    tab.inScope();
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<Block>") {
            gBlock(*it);
            tab.outScope();
        }
    }
    function.ret = ret;
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
        else if ((*it)->token.val == "[") ty.ptr = 1;
        else if ((*it)->token.val == "<ConstExp>") var.dims.push_back(gConstExp(*it));
    }
    if (ty.ptr == 1) var.dims.push_back(1);
    val.ty = ty;
    function.args.push_back(val);
    ty.ptr++;
    Value addr(newReg(), ty);
    Inst inst1(1, addr);
    inst1.name = "alloca";
    function.add(inst1);
    Inst inst2(2, val, addr);
    inst2.name = "store";
    function.add(inst2);
    var.reg = addr.reg;
    tab.add(var);
}

void gBlock(Node* node) {
    blocks++;
    for (auto it = node->sons.begin(); it != node->sons.end(); it++) {
        if ((*it)->token.val == "<BlockItem>") gBlockItem(*it);
    }
    blocks--;
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
            Inst inst(2, exp, lval);
            inst.name = "store";
            function.add(inst);
        }
        else if ((*(it+2))->token.val == "getint") {
            Value v(newReg(), i32);
            Function f = ir.getFunc("@getint");
            Inst call(2, v, f.ret);
            call.name = "call";
            function.add(call);
            Inst store(2, v, lval);
            store.name = "store";
            function.add(store);
        }
    }
    else if ((*it)->token.val == "<Exp>") gExp(*it);
    else if ((*it)->token.val == "<Block>") gBlock(*it);
    else if ((*it)->token.val == "return") {
        if ((*(it+1))->token.val == "<Exp>") {
            Value v = gExp(*(it+1));
            Inst inst(1, v);
            inst.name = "ret";
            function.add(inst);
        }
        else {
            Type ty("void", vector<int>(), 0);
            Value v(0, ty);
            Inst inst(1, v);
            inst.name = "ret";
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
        Function pc = ir.getFunc("@putch");
        for (int i = 1; i < s.size()-1; i++) {
            if (s[i] == '%') {
                Inst inst(2, pi.ret, vals[0]);
                inst.name = "call";
                function.add(inst);
                vals.erase(vals.begin());
                i++;
            }
            else if (s[i] == '\\') {
                Value c((int)'\n', i32);
                Inst inst(2, pc.ret, c);
                inst.name = "call";
                function.add(inst);
                i++;
            }
            else {
                Value c((int)s[i], i32);
                Inst inst(2, pc.ret, c);
                inst.name = "call";
                function.add(inst);
            }
        }
    }
}

Value gExp(Node* node) {
    return gAddExp(node->sons[0]);
}

Value gLVal(Node* node, bool isAddr) {
    Var var = tab.findVar(node->sons[0]->token.val);
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
            Inst inst(2, res, val);
            inst.name = "load";
            function.add(inst);
        }
        else {
            ty.shape.pop_back();
            res.ty = ty;
            Inst inst(4, res, val, Value(0, i32), Value(0, i32));
            inst.name = "getelementptr inbounds";
            function.add(inst);
        }
        return res;
    }
    if (val.ty.ptr == 2) {
        Type ty = val.ty;
        ty.ptr--;
        Value res(newReg(), ty);
        Inst inst(2, res, val);
        inst.name = "load";
        function.add(inst);
        val = res;
    }
    int num = var.flat()[0];
    Type ty = i32;
    Value v(num, ty);
    for (int i = 0; i < ind.size(); i++) {
        v.num /= var.dims[i];
        Value res(newReg(), ty);
        Inst inst(3, res, ind[i], v);
        inst.name = (i == 0) ? "mul" : "add";   /*   array dim leq 2   */
        function.add(inst);
        v = res;
    }
    ty = i32p;
    Value addr(newReg(), ty);
    Inst inst(2, addr, val);
    if (val.ty.shape.size() != 0) {         //is array
        inst.ops.push_back(Value(0, i32));  /*     array is different from pointer     */
    }
    inst.ops.push_back(v);
    inst.name = "getelementptr inbounds";
    function.add(inst);
    if (ind.size() < var.dims.size()) isAddr = true;
    if (isAddr) return addr; 
    ty.ptr--;
    Value res(newReg(), ty);
    Inst inst2(2, res, addr);
    inst2.name = "load";
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
        Inst inst(0);
        if (f.ret.ty.data == "i32") {
            Value res(newReg(), f.ret.ty);
            inst.ops.push_back(res);
        }
        inst.ops.push_back(f.ret);
        inst.name = "call";
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
            Value v1(0, v2.ty);
            Value res(newReg(), v2.ty);
            Inst inst(3, res, v1, v2);
            inst.name = "sub";
            function.add(inst);
            v2 = res;
        } 
        else if ((*it)->sons[0]->token.val == "!") {
            //TODO
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
            Value res(newReg(), v1.ty);
            Inst inst(3, res, v1, v2);
            inst.name = name;
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
            Value res(newReg(), v1.ty);
            Inst inst(3, res, v1, v2);
            inst.name = name;
            function.add(inst);
            v1 = res;
        }
    }
    return v1;
}

void generate(Node* root) {
    gCompUnit(root);
    FILE* fp = fopen("llvm_ir.txt", "w");
    printIR(fp);
    fclose(fp);
}

}