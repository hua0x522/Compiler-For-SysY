#include "lexer.h"
#include "analyser.h"
#include "check.h"
#include "generate.h"
#include "mem2reg.h"
#include "translate.h"

int main() {
    vector<Token> tokens = lexer();
    Node* root = LL(tokens);
    CHECK::check(root);
    IR ir = GEN::generate(root);
    mem2reg(ir);
    TRANS::translate(ir);
    return 0;
}