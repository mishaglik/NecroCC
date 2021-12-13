#include "Tree.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include <string.h>

const size_t INIT_LIST_SZ = 64;
const size_t EXPAND_COEF  = 2;

const int SIGNATURE = 0x72EE;

void nodeListCtor(NodeList* list){
    LOG_ASSERT(list != NULL);

    list->nodes = (Node*)mgk_calloc(INIT_LIST_SZ, sizeof(Node));
    list->capacity = INIT_LIST_SZ;
    list->size = 0;

    LOG_INFO("NodeList successfully created");
}

void nodeListDtor(NodeList* list){
    if(list == NULL) return;

    free(list->nodes);
    list->capacity = 0;
    list->size = 0;
    free(list);
}

Node* nodeListPush(NodeList* list, Node node){
    LOG_ASSERT(list != NULL);

    if(list->size + 2 >= list->capacity){
        list->nodes = (Node*)mgk_realloc(list->nodes, EXPAND_COEF * list->capacity, sizeof(Node));
        list->capacity = EXPAND_COEF * list->capacity;
    }

    list->nodes[list->size++] = node;
    return list->nodes + list->size - 1;
}

void graphTree(const Node* node){
    LOG_ASSERT(node != NULL);

    FILE* dotFile = fopen("/tmp/graph.dot", "w");
    
    fprintf(dotFile, "digraph G{\n");
    graphNode(node, dotFile);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    LOG_INFO("*Tree graph*");

    static int nGraph = 0;
    
    char command[50] = "";

    sprintf(command, "dot -T png /tmp/graph.dot -o log/GRAPH_NODE_%d.png", nGraph++);
    LOG_INFO("Executing command: \"%s\"", command);
    system(command);
}

void graphNode(const Node* node, FILE* graphFile){
    LOG_ASSERT(node      != NULL);
    LOG_ASSERT(graphFile != NULL);

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
                OP_CASE(OUT   );
                OP_CASE(IN   );
                OP_CASE(OUTC   );
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

Node newNode(NodeType type, NodeData data){
    return {type, data, NULL, NULL};
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

void writeTree(const Tree* tree, const char* filename){
    LOG_ASSERT(tree != NULL);
    LOG_ASSERT(tree->list != NULL);
    LOG_ASSERT(tree->root != NULL);

    FILE* file = fopen(filename, "wb");
    LOG_ASSERT(file != NULL);

    fwrite(&SIGNATURE, sizeof(int), 1, file);

    fwrite(&tree->list->size, sizeof(size_t), 1, file);
    long root_index = tree->root - tree->list->nodes;
    fwrite(&root_index, sizeof(long), 1, file);

    for(size_t i = 0; i < tree->list->size; ++i){
        Node node = tree->list->nodes[i];
        node.left  -= (long)tree->list->nodes;
        node.right -= (long)tree->list->nodes;
        fwrite(&node, sizeof(Node), 1, file);
    }

    fclose(file);
}

Tree* readTree(const char* filename){
    LOG_ASSERT(filename != NULL);

    FILE* file = fopen(filename, "rb");
    LOG_ASSERT(file != NULL);

    int sig = 0;
    fread(&sig, sizeof(int), 1, file);
    if(sig != SIGNATURE){
        LOG_ERROR("Incorrect tree file.");
        return NULL;
    }

    size_t size  = 0;
    long root_index = 0;

    if(fread(&size, sizeof(size_t), 1, file) == 0){
        LOG_ERROR("File too short");
        return NULL;
    }
    if(fread(&root_index, sizeof(long), 1, file) == 0){
        LOG_ERROR("File too short");
        return NULL;
    }

    Tree* tree = (Tree*)mgk_calloc(1, sizeof(Tree));
    tree->list = (NodeList*)mgk_calloc(1, sizeof(NodeList));
    tree->list->size = size;
    tree->list->capacity = tree->list->size;
    tree->list->nodes = (Node*)mgk_calloc(size, sizeof(Node));

    if(fread(tree->list->nodes, sizeof(Node), size, file) != size){
        LOG_ERROR("File too short");
        nodeListDtor(tree->list);
        free(tree);
        return NULL;
    }

    for(size_t i = 0; i < size; ++i){
        tree->list->nodes[i].left  += (long)tree->list->nodes;
        tree->list->nodes[i].right += (long)tree->list->nodes;
    }
    tree->root = tree->list->nodes + root_index;
    return tree;
}