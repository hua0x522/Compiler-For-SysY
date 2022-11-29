#include "table.h"

vector<int> Var::flat() {
    vector<int> v;
    if (dims.size() == 0) return v;
    int num = 1;
    for (int i = 0; i < dims.size(); i++) {
        num *= dims[i];
    }
    v.push_back(num);
    return v;
}

bool Table::has(string s) {
    bool inscope = false;
    for (int i = this->vars.size()-1; i >= 0; i--) {
        if (this->vars[i].id == "<BASE>") {
            inscope = true;
            break;
        }
        if (this->vars[i].id == s) return true;
    }
    if (!inscope) {
        for (int i = 0; i < this->funcs.size(); i++) {
            if (this->funcs[i].id == s) return true;
        }
    } 
    return false;
}

void Table::toString() {
    for (int i = 0; i < this->vars.size(); i++) {
        printf("%s ", this->vars[i].id.c_str());
    }
    printf("\n");
    for (int i = 0; i < this->funcs.size(); i++) {
        printf("%s ", this->funcs[i].id.c_str());
    }
    printf("\n");
}

void Table::add(Var var) {
    if (!this->has(var.id)) {
        this->vars.push_back(var);
    }
}

void Table::add(Func func) {
    for (int i = 0; i < this->funcs.size(); i++) {
        if (this->funcs[i].id == func.id) return;
    }
    this->funcs.push_back(func);
}

void Table::inScope() {
    Var var("<BASE>", "", 0);
    this->add(var);
}

void Table::outScope() {
    while ((*(this->vars.end()-1)).id != "<BASE>") {
        this->vars.pop_back();
    }
    this->vars.pop_back();
}

Var Table::findVar(string id) {
    for (int i = this->vars.size()-1; i >= 0; i--) {
        if (this->vars[i].id == id) return this->vars[i];
    }
    return Var("","",-1); 
}

Func Table::findFunc(string id) {
    for (int i = this->funcs.size()-1; i >= 0; i--) {
        if (this->funcs[i].id == id) return this->funcs[i];
    }
    Func func;
    func.id = "";
    return func; 
}