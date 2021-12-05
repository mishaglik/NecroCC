#include <stdio.h>
#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "File.h"
#include "FrontEnd/Parser/Parser.h"
#include "FrontEnd/SyntaxAnal/SyntaxAnalyzer.h"
int main(){
    FILE* file = fopen("Text.cht", "r");
    LOG_ASSERT(file != NULL);
    size_t file_sz = getFileSize(file);
    char* buffer = (char*) mgk_calloc(file_sz + 1, sizeof(char));
    fread(buffer, sizeof(char), file_sz, file);
    fclose(file);
    
    NodeList* list = parseText(buffer);
    for(size_t i = 0; i < list->size; ++i){
        char* label = getNodeLabel(list->nodes[i]);
        LOG_DEBUG("[%3d] = { .type = %d, .data = %d \t .label = %s}", i, (int)list->nodes[i]->type, list->nodes[i]->data.num, label);
        free(label);
    }

    SyntaxContext context = {
        .curPtr = list->nodes,
        .size   = list->size,
    };

    context = getG(context);

    graphTree(context.root);
    nodeListDtor(list);
    free(buffer);
    LOG_INFO("Successfully finished");
    LOG_FILLER();
    return 0;
}