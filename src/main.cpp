#include "lexer.h"
#include <map>
using namespace std;

map<string, string> typeMap;

string getType(string str) {
    if (typeMap.count(str)) {
        return typeMap[str];
    }
    else {
        if (str[0] == '\"') return "STRCON";
        else if (isDigit(str[0])) return "INTCON";
        else return "IDENFR";
    }
}

int main() {
    typeMap["main"] = "MAINTK";
    typeMap["const"] = "CONSTTK";
    typeMap["int"] = "INTTK";
    typeMap["break"] = "BREAKTK";
    typeMap["continue"] = "CONTINUETK";
    typeMap["if"] = "IFTK";    
    typeMap["else"] = "ELSETK";
    typeMap["!"] = "NOT";
    typeMap["&&"] = "AND";
    typeMap["||"] = "OR";
    typeMap["while"] = "WHILETK";
    typeMap["getint"] = "GETINTTK";
    typeMap["printf"] = "PRINTFTK";
    typeMap["return"] = "RETURNTK";
    typeMap["+"] = "PLUS";
    typeMap["-"] = "MINU";
    typeMap["void"] = "VOIDTK";
    typeMap["*"] = "MULT";
    typeMap["/"] = "DIV";   
    typeMap["%"] = "MOD"; 
    typeMap["<"] = "LSS";
    typeMap["<="] = "LEQ";
    typeMap[">"] = "GRE";
    typeMap[">="] = "GEQ";
    typeMap["=="] = "EQL";
    typeMap["!="] = "NEQ";
    typeMap["="] = "ASSIGN";
    typeMap[";"] = "SEMICN";
    typeMap[","] = "COMMA";
    typeMap["("] = "LPARENT";
    typeMap[")"] = "RPARENT";
    typeMap["["] = "LBRACK";
    typeMap["]"] = "RBRACK";
    typeMap["{"] = "LBRACE";
    typeMap["}"] = "RBRACE";    

    vector<string> tokens = lexer(); 
    FILE* fp = fopen("output.txt", "w");
    for (int i = 0; i < tokens.size(); i++) {
        fprintf(fp, "%s %s\n", getType(tokens[i]).c_str(), tokens[i].c_str());
    }
    fclose(fp);
    return 0;
}