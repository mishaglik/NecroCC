#include "MGK/Logger.h"
#include "MGK/Utils.h"

#include "Frontend.h"
#include "Parser/Parser.h"
#include "SyntaxAnal/SyntaxAnalyzer.h"

Tree* frontEnd(const char* str){
    LOG_ASSERT(str != NULL);
    // LOG_ASSERT(root != NULL);
    NodeList* list = parseText(str);

    Node node = newNode(NodeType::NONE, {});

    nodeListPush(list, node);

    LOG_DEBUG("Arr[%p]", list->nodes);
    for(size_t i = 0; i < list->size; ++i){
        char* label = getNodeLabel(list->nodes + i);
        LOG_DEBUG("[%3d] = { .type = %d, .data = %d \t .label = %s}", i, (int)list->nodes[i].type, list->nodes[i].data.num, label);
        free(label);
    }

    SyntaxContext context = {
        .curPtr = list->nodes,
        .size   = list->size,
        .start  = list->nodes,
    };

    context = getG(context);

    graphTree(context.root);
    Tree* tree = (Tree*)mgk_calloc(1, sizeof(Tree));
    tree->list = list;
    tree->root = context.root;
    return tree;
}

int main(int argc, char* argv[]){
    LOG_ASSERT(argc > 2);
    Tree* tree = frontEnd(argv[1]);
    LOG_ASSERT(tree != NULL);
    
    writeTree(tree, argv[2]);

    nodeListDtor(tree->list);
    free(tree);
    return 0;
}