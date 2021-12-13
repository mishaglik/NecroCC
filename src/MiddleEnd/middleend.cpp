#include "middleend.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"

void middleEnd(Tree* tree){
    LOG_ASSERT(tree != NULL);

    diffTree(tree->root, tree->list);
}

void diffTree(Node* node, NodeList* list){
    LOG_ASSERT(node != NULL);
    LOG_ASSERT(list != NULL);

    if(node->left)  diffTree(node->left , list);
    if(node->right) diffTree(node->right, list);

    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::DIFF){
        *node = diff(node, list);
    }
}

Node diff(Node* node, NodeList* list){
    LOG_ASSERT(list != NULL);
    if(node == NULL) return {};

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (node->type)
    {
    case NodeType::OPERATOR:
        {
            switch (node->data.opr)
            {
            
            case Operator::MUL:
                {
                    Node d_l = diff(node->left, list);
                    Node d_r = diff(node->right, list);

                    Node mul_l = {.type = NodeType::OPERATOR, .data = {.opr = Operator::MUL}, .left = nodeListPush(list, d_l), .right = node->right};
                    Node mul_r = {.type = NodeType::OPERATOR, .data = {.opr = Operator::MUL}, .left = nodeListPush(list, d_r), .right = node->left};

                    Node newNode = {.type = NodeType::OPERATOR,
                                    .data = {.opr = Operator::ADD},
                                    .left  = nodeListPush(list, mul_l),
                                    .right = nodeListPush(list, mul_r),
                                    };
                    return newNode;
                }
                break;
            case Operator::DIV:
                {
                    Node d_l = diff(node->left, list);
                    Node d_r = diff(node->right, list);

                    Node mul_l = {.type = NodeType::OPERATOR, .data = {.opr = Operator::MUL}, .left = nodeListPush(list, d_l), .right = node->right};
                    Node mul_r = {.type = NodeType::OPERATOR, .data = {.opr = Operator::MUL}, .left = nodeListPush(list, d_r), .right = node->left};

                    Node div = {.type = NodeType::OPERATOR, .data = {.opr = Operator::MUL}, .left = node->right, .right = node->right};

                    Node newNode = {.type = NodeType::OPERATOR,
                                    .data = {.opr = Operator::SUB},
                                    .left  = nodeListPush(list, mul_l),
                                    .right = nodeListPush(list, mul_r),
                                    };
                    newNode = {.type = NodeType::OPERATOR, 
                               .data = {.opr = Operator::DIV},
                               .left = nodeListPush(list, newNode),
                               .right = nodeListPush(list, div)};
                    return newNode;
                }
                break;
            case Operator::SUB:
            case Operator::ADD:
                {
                    Node newNode = *node;
                    node->left  = nodeListPush(list, diff(node->left , list));
                    node->right = nodeListPush(list, diff(node->right, list));
                    return newNode;
                }
                break;
            default:
                return *node;
            }
        }
    case NodeType::IDENTIFIER:
        {
        Node newNode = {.type=NodeType::NUMBER,.data={.num=1}};
        return newNode;
        }
    case NodeType::NUMBER:
        {
        Node newNode = {.type=NodeType::NUMBER,.data={.num=0}};
        return newNode;
        }
    case NodeType::NONE:
    case NodeType::CUSTOM:
    default:
        return *node;
    }
    #pragma GCC diagnostic pop

}

int main(int argc, char* argv[]){
    LOG_ASSERT(argc > 1);
    Tree* tree = readTree(argv[1]);
    middleEnd(tree);
    writeTree(tree, argv[1]);

    nodeListDtor(tree->list);
    free(tree);
}