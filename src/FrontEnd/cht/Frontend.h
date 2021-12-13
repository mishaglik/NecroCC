#ifndef NECROCC_FRONT_END_H
#define NECROCC_FRONT_END_H

#include "../../LangTree/Tree.h"

Tree* frontEnd(const char* str);

void frontEnd_reversed(const Tree* tree, const char* filename);

void printNode(const Node* node, FILE* file);

const char* getKeywordStr(const Node* node);

int getPriority(const Node* node);
#endif