#include "lexer.h"
#include "analyser.h"
#include "check.h"
#include "generate.h"

int main() {
    vector<Token> tokens = lexer();
    Node* root = LL(tokens);
    CHECK::check(root);
    GEN::generate(root);
    return 0;
}