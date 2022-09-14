#include "lexer.h"

void clearToken(vector<string>& tokens, vector<char>& token) {
    if (token.size() > 0) {
        string str(token.begin(), token.end());
        tokens.push_back(str);
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

vector<string> lexer() {
    vector<string> tokens;
    FILE* fp = fopen("testfile.txt", "r");
    vector<char> token;
    char ch = getc(fp);
    while(!feof(fp)) {
        if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
            clearToken(tokens, token);
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
            }
            else if (ch == '*') {
                int status = 0;
                while(1) {
                    ch = getc(fp);
                    if (status == 0) {
                        if (ch == '*') status = 1;
                    }
                    else {
                        if (ch == '/') break;
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