#include <stdio.h>
#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "File.h"
#include "FrontEnd/Frontend.h"
int main(){
    logger_set_file("ncc.log");
    FILE* file = fopen("Text.cht", "r");
    LOG_ASSERT(file != NULL);
    size_t file_sz = getFileSize(file);
    char* buffer = (char*) mgk_calloc(file_sz + 1, sizeof(char));
    fread(buffer, sizeof(char), file_sz, file);
    fclose(file);
    
    NodeList* list = frontEnd(buffer);

    nodeListDtor(list);
    free(buffer);
    LOG_INFO("Successfully finished");
    LOG_FILLER();
    return 0;
}