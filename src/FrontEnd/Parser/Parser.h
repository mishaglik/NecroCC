#ifndef NECROCC_FRONT_PARSER_PARSER_H
#define NECROCC_FRONT_PARSER_PARSER_H
#include "../../LangTree/Tree.h"

struct Keyword
{
    const char* string = NULL;
    NodeType type = NodeType::NONE;
    NodeData data = {0};
};

struct Identifier
{
    char* str = NULL;
    int   id  = 0;
};

const size_t MAX_TOKEN_LEN = 32;

NodeList* parseText(const char* text);

const Keyword* findKeyword(const char* word);

#endif