#include "lexer.h"
#include "analyser.h"
#include "check.h"
#include "generate.h"
#include "mem2reg.h"
#include "translate.h"

int main() {
    vector<Token> tokens = lexer();
//    printf("finish lexer\n");
    Node* root = LL(tokens);
//    printf("finish LL\n");
    CHECK::check(root);
    IR ir = GEN::generate(root);
//    printf("finish generate IR\n");
    mem2reg(ir);
//    printf("finish mem2reg\n");
//    TRANS::translate(ir);
//    printf("finish translate mips\n");
    return 0;
}