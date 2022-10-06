#include "lexer.h"
#include "token.h"
#include <map>

int line = 1;

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

void initMap() {
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
}

void clearToken(vector<Token>& tokens, vector<char>& token) {
    if (token.size() > 0) {
        string str(token.begin(), token.end());
        Token t(str, getType(str), line);
        tokens.push_back(t);
        token.clear();
    }
}

bool isCase(char ch) {
    if (ch >= 'a' && ch <= 'z') return true;
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch == '_') return true;
    return false;
}

bool isDigit(char ch) {
    return (ch >= '0' && ch <= '9');
}

bool compareOp(char ch) {
    return (ch == '<' || ch == '>' || ch == '!' || ch == '=');
}

bool separatorOp(char ch) {
    return (ch == ';' || ch == ',' || ch == '(' || ch == ')' || 
            ch == '[' || ch == ']' || ch == '{' || ch == '}');
}

bool isOp(char ch) {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '%');
}

vector<Token> lexer() {
    initMap();
    vector<Token> tokens;
    FILE* fp = fopen("testfile.txt", "r");
    vector<char> token;
    char ch = getc(fp);
    while(!feof(fp)) {
        if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
            clearToken(tokens, token);
            if (ch == '\n') line++;
        }
        else if (isCase(ch) || isDigit(ch)) {
            token.push_back(ch);
        }
        else if (ch == '\"') {
            clearToken(tokens, token);
            token.push_back(ch);
            do {
                ch = getc(fp);
                token.push_back(ch);
            } 
            while (ch != '\"');
            clearToken(tokens, token);
        }
        else if (ch == '|' || ch == '&') {
            clearToken(tokens, token);
            token.push_back(ch);
            ch = getc(fp);
            token.push_back(ch);
            clearToken(tokens, token);
        }
        else if (compareOp(ch)) {
            clearToken(tokens, token);
            token.push_back(ch);
            ch = getc(fp);
            if (ch == '=') {
                token.push_back(ch);
                clearToken(tokens, token);
            }
            else {
                clearToken(tokens, token);
                continue;
            }
        }
        else if (separatorOp(ch)) {
            clearToken(tokens, token);
            token.push_back(ch);
            clearToken(tokens, token);
        } 
        else if (isOp(ch)) {
            clearToken(tokens, token);
            token.push_back(ch);
            clearToken(tokens, token);
        }
        else if (ch == '/') {
            clearToken(tokens, token);
            ch = getc(fp);
            if (ch == '/') {
                while (ch != '\n') {
                    ch = getc(fp);
                }
                line++;
            }
            else if (ch == '*') {
                int status = 0;
                while(1) {
                    ch = getc(fp);
                    if (ch == '\n') line++;
                    if (status == 0) {
                        if (ch == '*') status = 1;
                    }
                    else {
                        if (ch == '/') break;
                        else if (ch == '*') status = 1;
                        else status = 0;
                    }
                }
            }
            else {
                token.push_back('/');
                clearToken(tokens, token);
                continue;
            }
        }
        ch = getc(fp);
    }
    fclose(fp);
    return tokens;
}