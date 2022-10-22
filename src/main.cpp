#include "lexer.h"
#include "analyser.h"
#include "check.h"

int main() {
    vector<Token> tokens = lexer();
    Node* root = LL(tokens);
    check(root);
    return 0;
}