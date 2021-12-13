#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "File.h"
#include <ctype.h>

#include "Frontend.h"
#include "Tokenisator/Tokenisator.h"
#include "Tokenisator/Keywords.h"
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

    if(context.nParsed < 0) {
        LOG_ERROR("Syntax error");
        nodeListDtor(list);
        return NULL;
    }

    Tree* tree = (Tree*)mgk_calloc(1, sizeof(Tree));
    tree->list = list;
    tree->root = context.root;
    return tree;
}

void frontEnd_reversed(const Tree* tree, const char* filename){
    LOG_ASSERT(tree != NULL);
    LOG_ASSERT(filename != NULL);

    FILE* file = fopen(filename, "w");
    LOG_ASSERT(file != NULL);

    printNode(tree->root, file);

    fclose(file);
}

void printNode(const Node* node, FILE* file){
    if(node == NULL) return;
    LOG_ASSERT(file != NULL);

    const Node par_l = {.type = NodeType::CUSTOM, .data = {.custom = CustomOperator::PAR_L}};
    const Node par_r = {.type = NodeType::CUSTOM, .data = {.custom = CustomOperator::PAR_R}};

    #define LEFT  printNode(node->left , file)
    #define RIGHT printNode(node->right, file)
    #define KEYWORD fprintf(file, "%s ", getKeywordStr(node))
    #define PAR_L  printNode(&par_l, file)
    #define PAR_R printNode(&par_r, file)
    #define PRIORITY(action, second) {if(getPriority(node) > getPriority(second)) {PAR_L;} action; if(getPriority(node) > getPriority(second)){PAR_R;}}

    switch (node->type)
    {
    case NodeType::OPERATOR:
        {
            switch (node->data.opr)
            {
            case Operator::ENDL:
            case Operator::COMMA:
                LEFT;
                KEYWORD;
                fprintf(file, "\n");
                RIGHT;
                break;
            case Operator::OUT:
                KEYWORD;
                RIGHT;
                break;
            case Operator::FUNC:
                KEYWORD;
                LEFT;
                RIGHT;
                break;
            case Operator::F_ARG:
                PAR_L;
                LEFT;
                PAR_R;
                KEYWORD;
                PAR_L;
                RIGHT;
                PAR_R;
                break;
            case Operator::VAR:
                KEYWORD;
                LEFT;
                if(node->right){
                    Node tmpNode = {NodeType::OPERATOR, {.opr = Operator::SET}};
                    fprintf(file, "%s ", getKeywordStr(&tmpNode));
                }
                PRIORITY(RIGHT, node->right);
                break;
            case Operator::SET:
                LEFT;
                KEYWORD;
                PRIORITY(RIGHT, node->right);
                break;
            case Operator::TERN_Q:
                PRIORITY(LEFT, node->left);
                KEYWORD;
                RIGHT;
                break;
            case Operator::TERN_C:
            case Operator::QQ:
            case Operator::LAND:
            case Operator::LOR:
            case Operator::GRTR:
            case Operator::LESS:
            case Operator::LESS_EQ:
            case Operator::GRTR_EQ:
            case Operator::NON_EQ:
            case Operator::EQUAL:
            case Operator::AND:
            case Operator::XOR:
            case Operator::OR:
            case Operator::SHL:
            case Operator::SHR:
            case Operator::ADD:
            case Operator::SUB:
            case Operator::MUL:
            case Operator::MOD:
            case Operator::DIV:
            case Operator::NOT:
            case Operator::ADDR:
            case Operator::VAL:
                PRIORITY(LEFT, node->left);
                KEYWORD;
                PRIORITY(RIGHT, node->right);
                break;
            case Operator::WHILE:
                KEYWORD;
                PAR_L;
                LEFT;
                PAR_R;
                PAR_L;
                RIGHT;
                PAR_R;
                break;
            case Operator::DIFF:
                KEYWORD;
                PAR_L;
                RIGHT;
                PAR_R;
                return ;
            case Operator::INC:
            case Operator::DEC:
                if(node->left){
                    LEFT;
                    KEYWORD;
                }
                else{
                    KEYWORD;
                    RIGHT;
                }
                break;
                return ;
            case Operator::CALL:
                LEFT;
                PAR_L;
                RIGHT;
                PAR_R;
                break;
            case Operator::IN:
                KEYWORD;
                break;
            case Operator::OUTC:
            case Operator::RET:
            case Operator::BREAK:
            case Operator::NONE:
            default:
                LOG_ERROR("Bad call");
                return;
            }
        }
        break;
    case NodeType::NUMBER:
        {
            char num[10] = "";
            sprintf(num, "%d", node->data.num);
            for(int i = 0; i < 10 && num[i]; ++i){
                if(!isdigit(num[i])){
                    LOG_ERROR("Bad tree");
                    return;
                }
                Node digitNode = {.type = NodeType::NUMBER, .data = {.num = num[i]-'0'}};
                fprintf(file, "%s ", getKeywordStr(&digitNode));
            }
        }
        break;
    case NodeType::IDENTIFIER:
        if(node->data.id < (int)VarNameList_sz)
            fprintf(file, "%s ", VarNameList[node->data.id]);
        else {
            LOG_WARNING("Not enoung var names");
            fprintf(file, "_%d", node->data.id);
        }
        break;
    case NodeType::CUSTOM:
        fprintf(file, "%s", getKeywordStr(node));
        break;
    case NodeType::NONE:
    default:
        LOG_ERROR("Bad tree");
        return;
    }
}

const char* getKeywordStr(const Node* node){
    LOG_ASSERT(node != NULL);
    for(int i = 0; Dictionary[i].type != NodeType::NONE; ++i){
        if(Dictionary[i].type == node->type && Dictionary[i].data.num == node->data.num) return Dictionary[i].string;
    }
    LOG_ERROR("Keyword not found");
    return NULL;
}


int main(int argc, char* argv[]){
    LOG_ASSERT(argc > 2);

    LOG_DEBUG("%d:", argc);
    for(int i = 0; i < argc; ++i){
        LOG_DEBUG("\t.[%d]=\"%s\"", i ,argv[i]);
    }
    if(argc > 3 && argv[1][0] == '-'){

        Tree* tree = readTree(argv[2]);
        LOG_ASSERT(tree != NULL);

        frontEnd_reversed(tree, argv[3]);

        nodeListDtor(tree->list);
        free(tree);
        LOG_FATAL("\b\n");
        return 0;
    }


    FILE* file = fopen(argv[1], "r");
    LOG_ASSERT(file != NULL);

    size_t file_sz = getFileSize(file);
    char* str = (char*)mgk_calloc(file_sz + 1, sizeof(char));
    fread(str, sizeof(char), file_sz, file);

    Tree* tree = frontEnd(str);
    if(tree == NULL) {
        free(str);
        LOG_FATAL("\b\n");
        return 1;
    }

    writeTree(tree, argv[2]);

    nodeListDtor(tree->list);
    free(tree);
    free(str);
    LOG_FATAL("\b\n");
    return 0;
}

int getPriority(const Node* node){
    if(node == NULL) return 1000;
    if(node->type == NodeType::OPERATOR){
        switch (node->data.opr)
        {
        case Operator::ENDL:
        case Operator::OUT:
            return 1;
        case Operator::FUNC:
        case Operator::VAR:
            return 2;
        case Operator::SET:
            return 3;
        case Operator::QQ:
        case Operator::TERN_C:
        case Operator::TERN_Q:
            return 4;
        case Operator::LAND:
        case Operator::LOR:
            return 5;
        case Operator::LESS:
        case Operator::GRTR:
        case Operator::GRTR_EQ:
        case Operator::LESS_EQ:
        case Operator::EQUAL:
        case Operator::NON_EQ:
            return 6;
        case Operator::AND:
        case Operator::XOR:
        case Operator::OR:
        case Operator::SHL:
        case Operator::SHR:
            return 7;
        case Operator::ADD:
        case Operator::SUB:
            return 8;
        case Operator::MUL:
        case Operator::MOD:
        case Operator::DIV:
            return 9;
        case Operator::NOT:
            return 10;
        case Operator::WHILE:
        case Operator::DIFF:
            return 11;
        case Operator::INC:
        case Operator::DEC:
            return 12;
        case Operator::ADDR:
        case Operator::VAL:
            return 13;
        case Operator::CALL:
        case Operator::IN:
            return 14;
        case Operator::F_ARG:
        case Operator::OUTC:
        case Operator::RET:
        case Operator::BREAK:
        case Operator::COMMA:
        case Operator::NONE:
        default:
            // LOG_WARNING("Bad operator for priority");
            return 1000;
        }
    }
    return 15;
}
