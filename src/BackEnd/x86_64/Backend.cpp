#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "Backend.h"
#include <stdarg.h>
#include <string.h>
const size_t INIT_SZ = 32;

const int INT_SZ = 8; //sizeof(int);


#define ASM(format, ...) {ByteBufferAppendf(&context->asmBuf, format "\n\t", ## __VA_ARGS__);}

NameSpace* createNS(NameSpace* parent = NULL){
    NameSpace* ns = (NameSpace*)mgk_calloc(1, sizeof(NameSpace));
    ns->parent = parent;
    ns->size  = 0;
    ns->varTable = (VarOffset*)mgk_calloc(32, sizeof(VarOffset));
    ns->pushed = 0;
    return ns; 
}

void backend(const Node* root, const char* filename){
    LOG_ASSERT(root != NULL);
    LOG_ASSERT(filename != NULL);

    LOG_FILLER("BACKEND START");


    BackendContext context = {createNS()};
    newByteBuffer(&context.asmBuf);
    newByteBuffer(&context.binBuf);
    newByteBuffer(&context.labelBuf);
    newByteBuffer(&context.funcLabelsBuf);

    ByteBufferAppendf(&context.asmBuf,
        ".intel_syntax noprefix\n"
        ".global _start\n"
        "_start:\n");
    
    codeGen(&context, root);

    deleteNS(context.ns);
    FILE* asmFile = fopen(filename, "w");
    LOG_ASSERT(asmFile != NULL);
    fwrite(context.asmBuf.buff, sizeof(char), context.asmBuf.size, asmFile);
    deleteByteBuffer(&context.asmBuf);
    deleteByteBuffer(&context.binBuf);
    deleteByteBuffer(&context.labelBuf);
    deleteByteBuffer(&context.funcLabelsBuf);
    fclose(asmFile);

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
    return -(int)(ns->parent->pushed * INT_SZ) + getOffset(ns->parent, id);
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
        XPUSH(Reg::RAX);
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

    unsigned l = 0;

    switch (node->type)
    {
    case NodeType::OPERATOR:
    {
        switch (node->data.opr)
        {
        #define CASE_BINARY(opr, OPR)       \
        case Operator::opr:                 \
            codeGen(context, node->left);   \
            codeGen(context, node->right);  \
            XPOP(Reg::RCX);                 \
            XPOP(Reg::RAX);                 \
            OPR(Reg::RAX, Reg::RCX);        \
            XPUSH(Reg::RAX);                \
            break
        #define CASE_CMP(opr, flags)        \
        case Operator::opr:                 \
            codeGen(context, node->left);   \
            codeGen(context, node->right);  \
            XPOP(Reg::RBX);                 \
            XPOP(Reg::RAX);                 \
            XXOR(Reg::RCX, Reg::RCX);       \
            XCMPRR(Reg::RAX, Reg::RBX);     \
            XSET(flags, Reg::RCX);          \
            XPUSH(Reg::RCX);                \
            break
        CASE_BINARY(ADD, XADD);
        CASE_BINARY(SUB, XSUB);
        CASE_BINARY(MUL, XMUL);
        CASE_BINARY(DIV, XDIV); 
        // CASE_BINARY(MOD, XMOD);
        CASE_BINARY(SHL, XSHL);
        CASE_BINARY(SHR, XSHR);

        CASE_CMP(LESS, Flags::L);
        CASE_CMP(LESS_EQ, Flags::LE);
        CASE_CMP(GRTR, Flags::G);
        CASE_CMP(GRTR_EQ, Flags::GE);
        CASE_CMP(EQUAL, Flags::E);
        CASE_CMP(NON_EQ, Flags::NE);

        

        CASE_BINARY(AND, XAND);
        CASE_BINARY(OR , XOR );
        CASE_BINARY(XOR, XXOR);
        case Operator::MOD:
            LOG_ASSERT(0);
            break;
        case Operator::SET:
            LOG_ASSERT(node->left != NULL);
            if(node->left->type == NodeType::IDENTIFIER){
                codeGen(context, node->right);
                XPOPMVC(getOffset(context->ns, node->left->data.id));
                XPUSHMVC(getOffset(context->ns, node->left->data.id));
            }
            else{
                LOG_ASSERT(node->left->type == NodeType::OPERATOR);
                LOG_ASSERT(node->left->data.opr == Operator::VAL);
                codeGen(context, node->right);
                codeGen(context, node->left->right);
                XPOP(Reg::RAX);
                XPOP(Reg::RBX);
                XMOVRMR(Reg::RBX, Reg::RAX);
                XPUSH(Reg::RAX);
            }
            break;
        case Operator::TERN_Q:
            LOG_ASSERT(node->right->data.opr ==Operator::TERN_C);
            
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            l = LabelReserve(context, 2);

            XJMP(Flags::NZ, l)
            codeGen(context, node->right->left);
            XJMP(Flags::ABS, l + 1);
            LabelRegister(context, l);
            codeGen(context, node->right->right);
            LabelRegister(context, l + 1);
            break;
        case Operator::QQ:
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            l = LabelReserve(context, 1);
            XJMP(Flags::NZ, l);
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            LabelRegister(context, l);
            XPUSH(Reg::RAX);
            break;
        case Operator::ENDL:
            codeGen(context, node->left);
            
            if(node->right){
                XPOP(Reg::RAX);
                codeGen(context, node->right);
            }
            break;
        case Operator::VAR:
            LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);
            registerVar(context->ns, node->left->data.id);
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XMOVVR(getOffset(context->ns,node->left->data.id), Reg::RAX);
            XPUSH(Reg::RAX);
            break;
        case Operator::FUNC:{
            l = LabelReserve(context, 2);
            LabelRegister(context, l);
            functionRegister(context, node->data.id, l, 0);
            XJMP(Flags::ABS, l + 1);
            NameSpace* oldNS = context->ns;
            context->ns = createNS();
            regVars(context->ns, node->right->left);
            registerVar(context->ns, -1);
            XPOPMVC(getOffset(context->ns, -1));            
            codeGen(context, node->right->right);

            XPUSHMVC(getOffset(context->ns, -1));
            XRET();
            LabelRegister(context, l + 1);
            XLEARV(Reg::RAX, l);
            XPUSH(Reg::RAX);
            deleteNS(context->ns);
            context->ns = oldNS;
            }
            break;
        case Operator::CALL:
            {
            XMOVRC(Reg::RAX, (int)(INT_SZ * context->ns->size));
            XSUB(Reg::RBP, Reg::RAX);
            
            NameSpace* oldNS = context->ns;
            context->ns = createNS(oldNS);

            int offset = 0;
            evaluteArguments(context, node->right, &offset);
            XCALL(functionGetL(node->left->data.id));

            deleteNS(context->ns);
            context->ns = oldNS;

            XMOVRC(Reg::RAX, (int)(INT_SZ * context->ns->size));
            XADD(Reg::RBP, Reg::RAX);
            }
            break;
        case Operator::WHILE:
            openNewNS(context);
            registerVar(context->ns, -2);
            
            l = LabelReserve(context, 2);

            LabelRegister(context, l);
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XJMP(Flags::Z  , l + 1);
            codeGen(context, node->right);
            XPOPMVC(getOffset(context->ns, -2));
            XJMP(Flags::ABS, l);
            LabelRegister(context, l + 1);
            XPUSHMVC(getOffset(context->ns, -2));
            closeNS(context);
            break;
        case Operator::LAND:
            l = LabelReserve(context, 2);
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XJMP(Flags::Z, l);

            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XJMP(Flags::Z, l);
           
            XMOVRC(Reg::RAX, 1);
            XPUSH(Reg::RAX);
            XJMP(Flags::ABS, l + 1);

            LabelRegister(context, l);
            XMOVRC(Reg::RAX, 0);
            XPUSH(Reg::RAX);
            LabelRegister(context, l + 1);
            break;
        case Operator::LOR:
            l = LabelReserve(context, 2);
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XJMP(Flags::NZ, l);

            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XJMP(Flags::NZ, l);
           
            XMOVRC(Reg::RAX, 0);
            XPUSH(Reg::RAX)
            XJMP(Flags::ABS, l + 1);

            LabelRegister(context, l);
            XMOVRC(Reg::RAX, 1);
            XPUSH(Reg::RAX);
            LabelRegister(context, l + 1);
            break;
        case Operator::NOT:
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            XMOVRC(Reg::RAX, 0);
            XSET(Flags::Z ,Reg::RAX);
            break;
        case Operator::INC:
            if(node->left){
                LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);

                XMOVRV(Reg::RAX, getOffset(context->ns, node->left->data.id));
                XPUSH(Reg::RAX);
                XINC(Reg::RAX);
                XMOVVR(getOffset(context->ns, node->left->data.id), Reg::RAX);
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(Reg::RAX, getOffset(context->ns, node->left->data.id));
                XINC(Reg::RAX);
                XMOVVR(getOffset(context->ns, node->left->data.id), Reg::RAX);
                XPUSH(Reg::RAX);
            }
            break;
        case Operator::DEC:
            if(node->left){
                LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);

                XMOVRV(Reg::RAX, getOffset(context->ns, node->left->data.id));
                XPUSH(Reg::RAX);
                XDEC(Reg::RAX);
                XMOVVR(getOffset(context->ns, node->left->data.id), Reg::RAX);
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(Reg::RAX, getOffset(context->ns, node->left->data.id));
                XDEC(Reg::RAX);
                XMOVVR(getOffset(context->ns, node->left->data.id), Reg::RAX);
                XPUSH(Reg::RAX);
            }
            break;
        case Operator::ADDR:
            XLEARV(Reg::RAX, getOffset(context->ns, node->right->data.id));
            XPUSH(Reg::RAX);
            break;
        case Operator::VAL:
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XMOVRRM(Reg::RAX, Reg::RAX);
            break;
        case Operator::IN:
            XIN();
            XPUSH(Reg::RAX);
            break;
        case Operator::OUT:
            codeGen(context, node->right);
            XPOP(Reg::RDI);
            XOUT();
            XPUSH(Reg::RAX);
            break;
        case Operator::BREAK:
        case Operator::RET:
        case Operator::OUTC:
            LOG_ERROR("Unsupported oprators. Please do not use them");
            return;
        case Operator::DIFF:
            LOG_ERROR("Undiffered tree.");
            return;
        case Operator::F_ARG:
        case Operator::COMMA:
        case Operator::TERN_C:
        case Operator::NONE:
        default:
            LOG_ERROR("Incoorect tree");
            break;
        }
    }
    break;
    case NodeType::NUMBER:
        XMOVRC(Reg::RAX, node->data.num);
        XPUSH(Reg::RAX);
        break;
    case NodeType::IDENTIFIER:
        XPUSHMVC(getOffset(context->ns, node->data.id));
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

    XSUB(Reg::RBP, (int)(INT_SZ * context->ns->size));
    
    NameSpace* oldNS = context->ns;
    context->ns = createNS(oldNS);
}

void closeNS(BackendContext* context){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(context->ns->parent != NULL);

    NameSpace* oldNS = context->ns->parent;
    deleteNS(context->ns);
    context->ns = oldNS;

    XADD(Reg::RBP, (int)(INT_SZ * context->ns->size));
}

int main(int argc, char* argv[]){
    LOG_ASSERT(argc > 2);
    Tree* tree = readTree(argv[1]);

    backend(tree->root, argv[2]);

    nodeListDtor(tree->list);
    free(tree);
}

void newByteBuffer(ByteBuffer* buf){
    LOG_ASSERT(buf != NULL);
    buf->buff = (char*) mgk_calloc(1024, sizeof(char));
    buf->capacity = 1024;
    buf->size     = 0;
}
void deleteByteBuffer(ByteBuffer* buffer){
    LOG_ASSERT(buffer != NULL);
    free(buffer->buff);
}
void ByteBufferAppend(ByteBuffer* buf, const char* data, size_t size){
    LOG_ASSERT(buf  != NULL);
    LOG_ASSERT(data != NULL);

    while(buf->size + size > buf->capacity){
        ByteBufferExpand(buf);
    }

    memcpy(buf->buff + buf->size, data, size);
    buf->size += size;
}
void ByteBufferAppendf(ByteBuffer* buf, const char* format, ...){
    char s[100];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(s, 100, format, args);
    LOG_ASSERT(size >= 0);
    va_end(args);
    ByteBufferAppend(buf, s, (size_t)size);
}


unsigned LabelReserve(BackendContext* context, unsigned n){
    LOG_ASSERT(context != NULL);
    unsigned l = context->labels;
    context->labels += n;
    while(context->labelBuf.capacity < context->labels * sizeof(size_t)){
        ByteBufferExpand(&context->labelBuf);
    }
    return l;
}
void LabelRegister(BackendContext* context, unsigned label){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(label < context->labels);
    ASM("L%u:", label);
    ((size_t*)context->labelBuf.buff)[label] = context->binBuf.size;
}

long long LabelGetOffs(BackendContext* context, unsigned label){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(label < context->labels);
    return ((long long*) context->labelBuf.buff)[label];
}

void functionRegister(BackendContext* context, int id, unsigned label, int nargs){
    FuncLable l = {.id = id, .label = label, .nArgs = nargs};
    ByteBufferAppend(&context->funcLabelsBuf, (const char*)&l, sizeof(FuncLable));
}
unsigned functionGetL(BackendContext* context, int id){
    FuncLable* funcs = (FuncLable*)context->funcLabelsBuf.buff;
    for(size_t i = 0; i < context->funcLabelsBuf.size / sizeof(FuncLable); ++i){
        if(funcs[i].id == id) return funcs[i].label;
    }
    return 0;
}

void ByteBufferExpand(ByteBuffer* buf){
    buf->buff = (char*)mgk_realloc(buf->buff, buf->capacity *= 2, sizeof(char));
}