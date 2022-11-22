#include "lexer.h"
#include "analyser.h"
#include "check.h"
#include "generate.h"
#include "translate.h"

int main() {
    vector<Token> tokens = lexer();
    Node* root = LL(tokens);
    CHECK::check(root);
    IR ir = GEN::generate(root);
    // TRANS::translate(ir);
    return 0;
}