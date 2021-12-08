#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include <string.h>
#include <ctype.h>

#include "Parser.h"
#include "Keywords.h"

#define SPACE_SYM " \t\v\n"

const size_t TABLE_INIT_SZ = 32;

NodeList* parseText(const char* text){
    LOG_ASSERT(text != NULL);

    LOG_INFO("Tokenisation started:");
    LOG_INC_TAB();
    
    Identifier* idTable = (Identifier*)mgk_calloc(TABLE_INIT_SZ, sizeof(Identifier));
    size_t      tabCap  = TABLE_INIT_SZ;
    int         curId   = 1;

    NodeList* list = (NodeList*)mgk_calloc(1, sizeof(NodeList));
    nodeListCtor(list);

    char token[MAX_TOKEN_LEN] = {};
    int curLen = 0;

    while(*text != '\0'){

        if(*text == '\''){
            const char* nxt = strpbrk(text + 1, "'.;:,?!()-" SPACE_SYM);
            if(!nxt){
                LOG_ERROR("Commenst started here: \"%.15s\" has no finish\n", text);
                LOG_STYLE(ConsoleStyle::RED);
                LOG_ERROR("\b\t\t           ~~~~~^");
            }
            text = nxt + (*nxt == '\'' ? 1 : 0);
            continue;
        }

        if(curLen == 4 && strncmp(token, "meow", 4) == 0){
            const char* nxt = strstr(text + 4, "meow");
            const char* nxt2 = strchr(text + 1, '\n');

            if(!nxt && !nxt2){
                LOG_ERROR("Commenst started here: \"%.15s\" has no finish\n", text);
                LOG_STYLE(ConsoleStyle::RED);
                LOG_ERROR("\b\t\t           ~~~~~^");
            }
            if(nxt && nxt < nxt2) text = nxt + 4;
            text = nxt2;
            continue;
        }

        if(!isalnum(*text) && !(*text == '_')){

            if(curLen != 0){
                const Keyword* key = findKeyword(token);
                if(key){
                    if(list->size && key->type == NodeType::NUMBER && list->nodes[list->size - 1]->type == NodeType::NUMBER){
                        list->nodes[list->size - 1]->data.num =  10 * list->nodes[list->size - 1]->data.num + key->data.num;
                    }
                    else{
                        nodeListPush(list, newNode(key->type, key->data));
                    }
                }
                else{
                    LOG_INFO("Found identifier %.*s. Registering... ", curLen, text - curLen);
                    
                    int found = 0;

                    for(int i = 1; i < curId; ++i){
                        if(strcmp(token, idTable[i].str) == 0){
                            LOG_INFO("\b Already exists!");
                            nodeListPush(list, newNode(NodeType::IDENTIFIER, {.id=idTable[i].id}));
                            found = 1;
                            break;
                        }
                    }
                    if(!found){
                        LOG_INFO("\b New!");
                        if(tabCap <= (size_t)curId){
                            idTable = (Identifier*)mgk_realloc(idTable, 2*tabCap , sizeof(Identifier));
                            tabCap  *= 2;
                        }
                        idTable[curId++] = {strdup(token), curId};
                        nodeListPush(list, newNode(NodeType::IDENTIFIER, {.id = curId-1}));
                    }

                    memset(token, 0, MAX_TOKEN_LEN);
                    curLen   = 0;
                    
                }

            }

            memset(token, 0, MAX_TOKEN_LEN);
            token[0] = *text;
            curLen   = 1;

            const Keyword* key = findKeyword(token);
            if(key){
                nodeListPush(list, newNode(key->type, key->data));
            }
            
            text++;
            memset(token, 0, MAX_TOKEN_LEN);
            curLen = 0;

            continue;
        }
        else{
            if(curLen == MAX_TOKEN_LEN){
                text++;
                continue;
            }
            token[curLen++] = *(text++);
        }
    }

    for(int i = 0; i < curId; ++i){
        free(idTable[i].str);
    }
    free(idTable);

    LOG_DEC_TAB();
    LOG_INFO("Tokenisation finished");
    return list;
}

const Keyword* findKeyword(const char* word){
    LOG_ASSERT(word != NULL);

    for(size_t i = 0; Dictionary[i].type != NodeType::NONE; i++){
        if(strcmp(word, Dictionary[i].string) == 0){
            LOG_INFO("Found Keyword: \"%s\"", word);
            return Dictionary + i;
        }
    }
    return NULL;
}