#ifndef NECROCC_FRONT_SYNTAX_ANALYZER_H
#define NECROCC_FRONT_SYNTAX_ANALYZER_H
#include "../../LangTree/Tree.h"

struct SyntaxContext{
    Node** curPtr = NULL;
    size_t size   = 0;

    int nParsed   = 0;
    Node* root    = NULL;   

    Node** start  = NULL; 
};

SyntaxContext getOpr(SyntaxContext context, Operator opr);

SyntaxContext getCustom(SyntaxContext context, CustomOperator opr);

SyntaxContext getOprLadder(SyntaxContext context,const Operator* oprRq, size_t n, SyntaxContext (*rule)(SyntaxContext context));

#define GRAMMAR_RULE(x) SyntaxContext get ## x (SyntaxContext context)

GRAMMAR_RULE(G);

GRAMMAR_RULE(Expr);

GRAMMAR_RULE(Line);

GRAMMAR_RULE(DeclF);

GRAMMAR_RULE(DeclV);

GRAMMAR_RULE(Asg);

GRAMMAR_RULE(Opt);

GRAMMAR_RULE(Lval);

GRAMMAR_RULE(Rval);

GRAMMAR_RULE(Sum);

GRAMMAR_RULE(Mul);

GRAMMAR_RULE(P);

GRAMMAR_RULE(Wh);

GRAMMAR_RULE(Basic);

GRAMMAR_RULE(Call);

GRAMMAR_RULE(Number);

GRAMMAR_RULE(Id);



#endif