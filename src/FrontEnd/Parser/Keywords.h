#ifndef NECROCC_FRONEND_PARSER_KEYWORDS_H
#define NECROCC_FRONEND_PARSER_KEYWORDS_H

#include "Parser.h"

#define KEYWORD(val, str) {str, NodeType::OPERATOR, {.opr = Operator::val}},
#define NUMBER(val, str) {str, NodeType::NUMBER, {.num = val}},

const Keyword Dictionary[] = {
    {"(", NodeType::CUSTOM, {.custom = CustomOperator::PAR_L}},
    {")", NodeType::CUSTOM, {.custom = CustomOperator::PAR_R}},

    NUMBER(0, "zhro")
    NUMBER(1, "Dagon")
    NUMBER(9, "Blahaj")

    KEYWORD(ADD, "ah")
    KEYWORD(SUB, "-")
    KEYWORD(MUL, "ch")
    KEYWORD(DIV, "d")
    KEYWORD(MOD, "md")
    KEYWORD(AND, "goka")
    KEYWORD(OR , "r")
    KEYWORD(XOR, "xor")
    KEYWORD(SHL, "llll")
    KEYWORD(SHR, "nafl")

    KEYWORD(SET, "fhtagn")

    KEYWORD(EQUAL , "eq")
    KEYWORD(NON_EQ, "nq")
    KEYWORD(LESS,   "lprimo")
    KEYWORD(GRTR,   "gr")
    KEYWORD(LESS_EQ,"lqst")
    KEYWORD(GRTR_EQ,"grrr")

    KEYWORD(NOT, "nog")
    KEYWORD(INC, "isc")
    KEYWORD(DEC, "dh")

    KEYWORD(VAR, "Shoggoth")
    KEYWORD(FUNC, "Great")
    KEYWORD(F_ARG, "faaa")
    KEYWORD(COMMA, ",")
    KEYWORD(RET, "RT")

    KEYWORD(ENDL, ".")
    KEYWORD(QQ, "Hastur")
    KEYWORD(TERN_Q, "Azathoth")
    KEYWORD(TERN_C,":")

    KEYWORD(WHILE, "Nyarlathotep")


    {NULL, NodeType::NONE, {0}}    //End of array identifier
};

#undef KEYWORD
#undef NUMBER

#endif