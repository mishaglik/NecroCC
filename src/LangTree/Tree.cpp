#include "Tree.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include <string.h>

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

    if(list->size + 2 >= list->capacity){
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

    sprintf(command, "dot -T png /tmp/graph.dot -o log/GRAPH_NODE_%d.png", nGraph++);
    LOG_INFO("Executing command: \"%s\"", command);
    system(command);
}

void graphNode(const Node* node, FILE* graphFile){
    LOG_ASSERT(node      != NULL);
    LOG_ASSERT(graphFile != NULL);

    LOG_INFO("*Tree graph*");
    
    char* label = getNodeLabel(node);
    fprintf(graphFile, "N%p[shape=record, color=\"%s\",label=\" %s | {<l> %p| <r> %p} \"]\n", node, getTypeColor(node->type), label , node->left, node->right);
    free(label);

    if(node->left)
        graphNode(node->left, graphFile);
    if(node->right)
        graphNode(node->right, graphFile);
    
    if(node->left)
        fprintf(graphFile, "N%p->N%p\n", node, node->left);
    if(node->right)
        fprintf(graphFile, "N%p->N%p\n", node, node->right);

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

#define OP_CASE(x) case Operator::x: return strcpy(str, #x)

char* getNodeLabel(const Node* node){
    LOG_ASSERT(node != NULL);

    char* str = (char*)mgk_calloc(15, sizeof(char));

    switch (node->type)
    {
    case NodeType::NUMBER:
        sprintf(str, "%d", node->data.num);
        break;
    case NodeType::IDENTIFIER:
        sprintf(str, "#%d", node->data.id);
        break;
    case NodeType::CUSTOM:
        {
            switch (node->data.custom)
            {
            case CustomOperator::PAR_L:
                strcpy(str, "(");
                break;
            case CustomOperator::PAR_R:
                strcpy(str, ")");
                break;
            case CustomOperator::NONE:
            default:
                break;
            }
        }
        break;
    case NodeType::OPERATOR:
        {
            switch (node->data.opr)
            {
                OP_CASE(NONE    );
                OP_CASE(ADD     );
                OP_CASE(SUB     );
                OP_CASE(MUL     );
                OP_CASE(DIV     );
                OP_CASE(MOD     );
                OP_CASE(AND     );
                OP_CASE(OR      );
                OP_CASE(XOR     );
                OP_CASE(SHL     );
                OP_CASE(SHR     );
                OP_CASE(SET     );
                OP_CASE(EQUAL   );
                OP_CASE(NON_EQ  );
                OP_CASE(LESS    );
                OP_CASE(GRTR    );
                OP_CASE(LESS_EQ );
                OP_CASE(GRTR_EQ );
                OP_CASE(LOR     );
                OP_CASE(LAND    );
                OP_CASE(NOT     );
                OP_CASE(INC     );
                OP_CASE(DEC     );
                OP_CASE(VAR     );
                OP_CASE(FUNC    );
                OP_CASE(COMMA   );
                OP_CASE(CALL    );
                OP_CASE(ENDL    );
                OP_CASE(QQ      );
                OP_CASE(TERN_Q  );
                OP_CASE(TERN_C  );
                OP_CASE(WHILE   );
                OP_CASE(BREAK   );
                OP_CASE(ADDR    );
                OP_CASE(VAL     );
                OP_CASE(DIFF    );
                OP_CASE(RET     );
                OP_CASE(F_ARG   );
            default:
                break;
            }
        }
        break;
    case NodeType::NONE:
    default:
        break;
    }

    return str;
}
#undef OP_CASE
void graphNodeList(NodeList* list);

Node* newNode(NodeType type, NodeData data){
    Node* ptr = (Node*)mgk_calloc(1, sizeof(Node));
    ptr->type = type;
    ptr->data = data;
    return ptr;
}

void nodeDump(const Node* node){
    if(node == NULL){
        LOG_INFO("Node: NULL");
        return;
    }
    char* nodeStr = getNodeLabel(node);
    LOG_INFO("Node[%p]: {.type = %d, .data = %8d, .label = %s}", node,node->type, node->data.num, nodeStr);
    free(nodeStr);
}