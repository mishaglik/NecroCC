#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "File.h"

#include "Frontend.h"
#include "Tokenisator/Tokenisator.h"
#include "SyntaxAnal/SyntaxAnalyzer.h"

Tree* frontEnd(const char* str){
    LOG_ASSERT(str != NULL);
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

    FILE* file = fopen(argv[1], "r");
    LOG_ASSERT(file != NULL);

    size_t file_sz = getFileSize(file);
    char* str = (char*)mgk_calloc(file_sz + 1, sizeof(char));
    fread(str, sizeof(char), file_sz, file);

    Tree* tree = frontEnd(str);
    LOG_ASSERT(tree != NULL);

    writeTree(tree, argv[2]);

    nodeListDtor(tree->list);
    free(tree);
    free(str);
    return 0;
}