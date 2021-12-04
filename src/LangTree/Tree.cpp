#include "Tree.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"


const size_t INIT_LIST_SZ = 64;
const size_t EXPAND_COEF  = 2;

void nodeListCtor(NodeList* list){
    LOG_ASSERT(list != NULL);

    list->nodes = (Node**)mgk_calloc(INIT_LIST_SZ, sizeof(Node*));
    list->capacity = INIT_LIST_SZ;
    list->size = 0;

    LOG_INFO("NodeList successfully created");
}

void nodeListDtor(NodeList* list){
    if(list == NULL) return;

    for(size_t i = 0; i < list->size; ++i){
        if(list->nodes[i]){
            free(list->nodes[i]);
        }
    }

    free(list->nodes);
    list->capacity = 0;
    list->size = 0;
    free(list);
}

void nodeListPush(NodeList* list, Node* node){
    LOG_ASSERT(list != NULL);
    LOG_ASSERT(node != NULL);

    if(list->size + 2 <= list->capacity){
        list->nodes = (Node**)mgk_realloc(list->nodes, EXPAND_COEF * list->capacity, sizeof(Node*));
        list->capacity = EXPAND_COEF * list->capacity;
    }

    list->nodes[list->size++] = node;
}

void graphTree(const Node* node){
    LOG_ASSERT(node != NULL);

    FILE* dotFile = fopen("/tmp/graph.dot", "w");
    
    fprintf(dotFile, "digraph G{\n");
    graphNode(node, dotFile);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    static int nGraph = 0;
    
    char command[50] = "";

    sprintf(command, "dot -T png /tmp/graph.dot -o /log/GRAPH_NODE_%d", nGraph++);
    LOG_INFO("Executing command: \"%s\"", command);
    system(command);
}

void graphNode(const Node* node, FILE* graphFile){
    LOG_ASSERT(node      != NULL);
    LOG_ASSERT(graphFile != NULL);

    LOG_INFO("*Tree graph*");
    // fprintf(graphFile, "N%p[style=record, color=\"%s\",label=\" %s | {<l> %p| <r> %p} \"]", node, getTypeColor(node->type), );
}


const char* getTypeColor(NodeType type){
    const char* color = NULL;
    switch (type)
    {
    case NodeType::NUMBER:
        color = "green";
        break;

    case NodeType::IDENTIFIER:
        color = "green";
        break;
        
    case NodeType::OPERATOR:
        color = "blue";
        break;

    case NodeType::CUSTOM:
        color = "red";
        break;

    case NodeType::NONE:
    default:
        color = "black";
        break;
    }
    return color;
}

void graphNodeList(NodeList* list);

Node* newNode(NodeType type, NodeData data){
    Node* ptr = (Node*)mgk_calloc(1, sizeof(Node));
    ptr->type = type;
    ptr->data = data;
    return ptr;
}