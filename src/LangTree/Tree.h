#ifndef LANG_TREE_TREE_H
#define LANG_TREE_TREE_H
#include <stdlib.h>
#include <stdio.h>

enum class CustomOperator{
    NONE  = 0,
    PAR_L = 1,
    PAR_R = 2,
};

#define CUSTOM_NODE_DATA_TYPE CustomOperator

#include "LangTree.h"

struct NodeList
{
    size_t capacity = 0;
    size_t size     = 0;
    Node* nodes    = NULL;
};

struct Tree
{
    NodeList* list = NULL;
    Node*     root = NULL;
};


Node newNode(NodeType type, NodeData data);

void nodeListCtor(NodeList* list);

void nodeListDtor(NodeList* list);

Node* nodeListPush(NodeList* list, Node node);

void graphTree(const Node* node);

void graphNode(const Node* node, FILE* graphFile);

const char* getTypeColor(NodeType type);

char* getNodeLabel(const Node* node);

void nodeDump(const Node* node);

void writeTree(const Tree* tree, const char* filename);

Tree* readTree(const char* filename);

// void graphNodeList(NodeList* list);
#endif