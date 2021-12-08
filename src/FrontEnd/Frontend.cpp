#include "MGK/Logger.h"
#include "MGK/Utils.h"

#include "Frontend.h"
#include "Parser/Parser.h"
#include "SyntaxAnal/SyntaxAnalyzer.h"

NodeList* frontEnd(const char* str, Node** root){
    LOG_ASSERT(str != NULL);
    LOG_ASSERT(root != NULL);
    NodeList* list = parseText(str);

    Node* node = newNode(NodeType::NONE, {});

    nodeListPush(list, node);
    for(size_t i = 0; i < list->size; ++i){
        char* label = getNodeLabel(list->nodes[i]);
        LOG_DEBUG("[%3d] = { .type = %d, .data = %d \t .label = %s}", i, (int)list->nodes[i]->type, list->nodes[i]->data.num, label);
        free(label);
    }

    SyntaxContext context = {
        .curPtr = list->nodes,
        .size   = list->size,
        .start  = list->nodes,
    };

    context = getG(context);
    *root = context.root;

    graphTree(context.root);

    return list;
}