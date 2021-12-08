#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "Backend.h"

const size_t INIT_SZ = 32;

const int INT_SZ = sizeof(int);

#define ASM(format, ...) {for(int _ = 0; _ < context->nTabs; ++_){fprintf(context->asmFile, "\t");} fprintf(context->asmFile, format "\n", ## __VA_ARGS__);}

NameSpace* createNS(NameSpace* parent = NULL){
    NameSpace* ns = (NameSpace*)mgk_calloc(1, sizeof(NameSpace));
    ns->parent = parent;
    ns->size  = 0;
    ns->varTable = (VarOffset*)mgk_calloc(32, sizeof(VarOffset));

    return ns; 
}

void backend(const Node* root, const char* filename){
    LOG_ASSERT(root != NULL);
    LOG_ASSERT(filename != NULL);

    LOG_FILLER("BACKEND START");

    FILE* file = fopen(filename, "w");
    LOG_ASSERT(file != NULL);

    BackendContext context = {createNS(), file};

    fprintf(context.asmFile, "aip END_OF_FILE:\n");
    fprintf(context.asmFile, "pop bx\n");
    
    codeGen(&context, root);

    fprintf(context.asmFile, "out\n");
    fprintf(context.asmFile, "hlt\n");
    fprintf(context.asmFile, "END_OF_FILE:\n");


    deleteNS(context.ns);
    fclose(context.asmFile);

    LOG_FILLER("BACKEND END");
}

void deleteNS(NameSpace* ns){
    if(!ns) return;
    free(ns->varTable);
    ns->capacity = 0;
    free(ns);
}

int getOffset(NameSpace* ns, idt_t id){
    LOG_ASSERT(ns != NULL);
    LOG_ASSERT(ns->varTable != NULL);

    for(size_t i = 0; i < ns->size; ++i){
        if(ns->varTable[i].id == id){
            return ns->varTable[i].offset;
        }
    }
    if(ns->parent == NULL){
        LOG_ERROR("");
        LOG_STYLE(ConsoleStyle::RED);
        LOG_ERROR("\bVAR %d not registered", id);
        return -1;
    }
    return -(int)(ns->parent->size * INT_SZ) + getOffset(ns->parent, id);
}

void registerVar(NameSpace* ns, idt_t id){
    LOG_ASSERT(ns != NULL);
    LOG_ASSERT(ns->varTable != NULL);

    if(ns->size >= ns->capacity){
        expandNS(ns, ns->capacity * 2);
    }

    ns->varTable[ns->size] = {id, (int)(ns->size * INT_SZ)};
    ns->size++;
}

void expandNS(NameSpace* ns, size_t newSZ){
    LOG_ASSERT(ns != NULL);
    if(newSZ <= ns->size) return;

    ns->varTable = (VarOffset*)mgk_realloc(ns->varTable, newSZ, sizeof(VarOffset));

    ns->capacity = newSZ;
}

void codeGen(BackendContext* context ,const Node* node){
    LOG_ASSERT(context != NULL);
    context->nTabs++;

    if(node == NULL){
        ASM("push 0; NULL node");
        LOG_DEC_TAB();
        LOG_INFO("Finished");
        context->nTabs--;
        return;
    }

    char* label = getNodeLabel(node);
    LOG_INFO("Code generating for node[%p]: %s", node, label);
    ASM(";%s", label);
    free(label);
    LOG_INC_TAB();

    switch (node->type)
    {
    case NodeType::OPERATOR:
    {
        switch (node->data.opr)
        {
        #define CASE_BINARY(opr, asm)       \
        case Operator::opr:                 \
            codeGen(context, node->left);   \
            codeGen(context, node->right);  \
            ASM(asm);                       \
            break                          
        CASE_BINARY(ADD, "add");
        CASE_BINARY(SUB, "sub");
        CASE_BINARY(MUL, "mul");
        CASE_BINARY(DIV, "div");
        case Operator::SET:
            LOG_ASSERT(node->left != NULL);
            LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);
            codeGen(context, node->right);
            ASM("pop [bx+%d]; SET %d", getOffset(context->ns, node->left->data.id) , node->left->data.id);
            ASM("push [bx+%d];", getOffset(context->ns, node->left->data.id));

            break;
        case Operator::TERN_Q:
            LOG_ASSERT(node->right->data.opr ==Operator::TERN_C);
            codeGen(context, node->left);
            ASM("push 0");

            ASM("je ELSE_%p:", node);
            codeGen(context, node->right->left);

            ASM("jmp END_IF_%p:", node);
            ASM("ELSE_%p:", node);

            codeGen(context, node->right->right);
            ASM("END_IF_%p:", node);
            break;
        case Operator::QQ:
            codeGen(context, node->left);
            ASM("pop dx");
            ASM("push dx");
            ASM("push dx");
            ASM("push 0");
            ASM("je QQ_END_%p:", node);
            ASM("pop dx");
            codeGen(context, node->right);
            ASM("QQ_END_%p:", node);
            break;
        case Operator::ENDL:
            codeGen(context, node->left);
            
            if(node->right){
                ASM("pop dx; Empty pop");
                codeGen(context, node->right);
            }
            break;
        case Operator::VAR:
            LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);
            registerVar(context->ns, node->left->data.id);
            codeGen(context, node->right);
            ASM("pop [bx+%d]", getOffset(context->ns,node->left->data.id));
            ASM("push[bx+%d]", getOffset(context->ns,node->left->data.id));
            break;
        case Operator::FUNC:{
            ASM("jmp FUNC_END_%d:", node->left->data.id);
            ASM("FUNC_BEG_%d:",     node->left->data.id);

            NameSpace* oldNS = context->ns;
            context->ns = createNS();
            regVars(context->ns, node->right->left);
            registerVar(context->ns, -1);
            ASM("pop [bx+%d]", getOffset(context->ns, -1));
            
            codeGen(context, node->right->right);

            ASM("push [bx+%d]", getOffset(context->ns, -1));
            ASM("ret");
            ASM("FUNC_END_%d:", node->left->data.id);
            ASM("push bx + %d", node->left->data.id);
            deleteNS(context->ns);
            context->ns = oldNS;
            }
            break;
        case Operator::CALL:
            {
            ASM("push bx");
            ASM("push %d", (int)(INT_SZ * context->ns->size));
            ASM("add");
            ASM("pop bx");
            
            NameSpace* oldNS = context->ns;
            context->ns = createNS(oldNS);

            int offset = 0;
            evaluteArguments(context, node->right, &offset);
            ASM("call FUNC_BEG_%d:", node->left->data.id);

            deleteNS(context->ns);
            context->ns = oldNS;

            ASM("push bx");
            ASM("push %d", (int)(INT_SZ * context->ns->size));
            ASM("sub");
            ASM("pop bx");
            }
            break;
        case Operator::NONE:
        default:
            LOG_ERROR("Incoorect tree");
            break;
        }
    }
    break;
    case NodeType::NUMBER:
        ASM("push %d; %d", node->data.num, node->data.num);
        break;
    case NodeType::IDENTIFIER:
        ASM("push [bx+%d]; var %d", getOffset(context->ns, node->data.id), node->data.id);
        break;
    case Operator::WHILE:
        openNewNS(context);
        registerVar(context->ns, -2);

        ASM("WHILE_BEG_%p:", node);
        codeGen(context, node->left);
        ASM("push 0");
        ASM("je WHILE_END_%p", node);
        codeGen(context, node->right);
        ASM("pop [bx+%d]", getOffset(context->ns, -2));
        ASM("jmp WHILE_BEG_%p:", node);
        ASM("WHILE_END_%p:", node);
        ASM("push [bx+%d]", getOffset(context->ns, -2));
        closeNS(context);
        break;
    case NodeType::CUSTOM:
    case NodeType::NONE:
    default:
        LOG_ERROR("Incorrect tree!");
        abort();
        break;
    }
    LOG_DEC_TAB();
    context->nTabs--;
    LOG_INFO("Finished");
}

int getNargs(const Node* node){
    if(node == NULL)
        return 0;
    if(node->type == NodeType::IDENTIFIER)
        return 1;

    int ans = 1;
    while(node->right){
        ans++;
        node = node->right;
    }
    return ans;
}

void regVars(NameSpace* ns, const Node* node){
    if(node == NULL){
        return;
    }
    if(node->type == NodeType::IDENTIFIER){
        registerVar(ns, node->data.id);
        return;
    }
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::COMMA){
        regVars(ns, node->left);
        regVars(ns, node->right);
        return;
    }
    LOG_ERROR("Incorrect expr tree");
}

void evaluteArguments(BackendContext* context, const Node* node, int* offset){
    LOG_ASSERT(context != NULL);
    if(node == NULL) return;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::COMMA){
        evaluteArguments(context, node->left, offset);
        evaluteArguments(context, node->right, offset);
        return;
    }
    else{
        codeGen(context, node);
        ASM("pop [bx+%d]", *offset);
        *offset += INT_SZ;
    }
}

void openNewNS(BackendContext* context){
    LOG_ASSERT(context != NULL);

    ASM("push bx");
    ASM("push %d", (int)(INT_SZ * context->ns->size));
    ASM("add");
    ASM("pop bx");
    
    NameSpace* oldNS = context->ns;
    context->ns = createNS(oldNS);
}

void closeNS(BackendContext* context){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(context->ns->parent != NULL);

    NameSpace* oldNS = context->ns->parent;
    deleteNS(context->ns);
    context->ns = oldNS;

    ASM("push bx");
    ASM("push %d", (int)(INT_SZ * context->ns->size));
    ASM("sub");
    ASM("pop bx");   
}