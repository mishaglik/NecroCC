#ifndef NECROCC_MIDDLEEND_MIDDLEEND_H
#define NECROCC_MIDDLEEND_MIDDLEEND_H
#include "../LangTree/Tree.h"

void middleEnd(Tree* tree);

void diffTree(Node* node, NodeList* list);

Node diff(Node* node, NodeList* list);

#endif
