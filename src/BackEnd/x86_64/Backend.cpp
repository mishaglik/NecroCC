#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "Backend.h"
#include <stdarg.h>
#include <string.h>
const size_t INIT_SZ = 32;

const int INT_SZ = 8; //sizeof(int);


#define ASM(format, ...) {ByteBufferAppendf(&context->asmBuf, format "\n\t", ## __VA_ARGS__);}

const Reg stdcallRegs[6] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};

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


    BackendContext context = {};
    newByteBuffer(&context.asmBuf);
    newByteBuffer(&context.binBuf);
    newByteBuffer(&context.labelBuf);
    newByteBuffer(&context.funcLabelsBuf);

    ByteBufferAppendf(&context.asmBuf,
        ".intel_syntax noprefix\n"
        ".global main\n"
        "main:\n");
    
    LabelReserve(&context, 1);  // Take label 0;
    createFrame(&context, NULL, root);
    codeGen(&context, root);
    closeFrame(&context);
    xExit(&context);

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
    return (int)((ns->parent->above + ns->parent->pushed + 1) * INT_SZ) + getOffset(ns->parent, id);
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
    ASM(";//%s", label);
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
        CASE_BINARY(SHL, XSHL);
        CASE_BINARY(SHR, XSHR);
        
        case Operator::DIV:
            codeGen(context, node->left);
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XCQO();
            XPOP(Reg::RBX);
            XDIV(Reg::RBX);
            XPUSH(Reg::RAX);
            break;
        case Operator::MOD:
            codeGen(context, node->left);
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            XCQO();
            XPOP(Reg::RBX);
            XDIV(Reg::RBX);
            XPUSH(Reg::RDX);
            break;

        CASE_CMP(LESS, Flags::L);
        CASE_CMP(LESS_EQ, Flags::LE);
        CASE_CMP(GRTR, Flags::G);
        CASE_CMP(GRTR_EQ, Flags::GE);
        CASE_CMP(EQUAL, Flags::E);
        CASE_CMP(NON_EQ, Flags::NE);

        

        CASE_BINARY(AND, XAND);
        CASE_BINARY(OR , XOR );
        CASE_BINARY(XOR, XXOR);
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
            LOG_ASSERT(node->right->data.opr == Operator::TERN_C);
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            l = LabelReserve(context, 2);

            XJMP(Flags::Z, l);
            
            createFrame(context, context->ns, node);
            codeGen(context, node->right->left);
            XPOP(Reg::RAX);
            closeFrame(context);

            XJMP(Flags::ABS, l + 1);
            LabelRegister(context, l);
            
            createFrame(context, context->ns, node);
            codeGen(context, node->right->right);
            XPOP(Reg::RAX);
            closeFrame(context);
            
            LabelRegister(context, l + 1);
            XPUSH(Reg::RAX);
            break;
        case Operator::QQ:
            codeGen(context, node->left);
            XPOP(Reg::RAX);
            XTESTRR(Reg::RAX, Reg::RAX);
            l = LabelReserve(context, 1);
            XJMP(Flags::NZ, l);
            
            createFrame(context, context->ns, node);
            codeGen(context, node->right);
            XPOP(Reg::RAX);
            closeFrame(context);
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
            functionRegister(context, node->left->data.id, l, 0);
            XJMP(Flags::ABS, l + 1);
            LabelRegister(context, l);
            size_t of1 = context->binBuf.size;
            NameSpace* oldNS = context->ns;

            createFrame(context, NULL, node);
            functionEntryVars(context, node->right->left, NULL);

            codeGen(context, node->right->right);
            XPOP(Reg::RAX);
            closeFrame(context);
            context->ns = oldNS;
            XRET();

            LabelRegister(context, l + 1);
            *(int*)(&context->binBuf.buff[of1 - 4]) = (int)(context->binBuf.size - of1);
            XXOR(Reg::RAX, Reg::RAX);
            XPUSH(Reg::RAX);
            }
            break;
        case Operator::CALL:
            {
            int n = evaluteArguments(context, node->right);
            for(int i = 0; i < n; ++i){
                XPOP(stdcallRegs[i]);
            }
            XCALL(functionGetL(context, node->left->data.id));
            XPUSH(Reg::RAX);
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
            XPUSH(Reg::RAX);
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

int getNfuncArgs(const Node* node){
    if(node == NULL)
        return 0;
    if(node->type == NodeType::IDENTIFIER){
        LOG_WARNING("Found var [%p] #%d", node, node->data.id);
        return 1;
    }
    return getNfuncArgs(node->left) + getNfuncArgs(node->right);
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

int evaluteArguments(BackendContext* context, const Node* node){
    LOG_ASSERT(context != NULL);
    if(node == NULL) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::COMMA){
        return
        evaluteArguments(context, node->left) + 
        evaluteArguments(context, node->right);
    }
    else{
        codeGen(context, node);
        return 1;
    }
}

void openNewNS(BackendContext* context){
    LOG_ASSERT(context != NULL);

    XSUBRC(Reg::RBP, (int)(INT_SZ * context->ns->size));
    
    NameSpace* oldNS = context->ns;
    context->ns = createNS(oldNS);
}

void closeNS(BackendContext* context){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(context->ns->parent != NULL);

    NameSpace* oldNS = context->ns->parent;
    deleteNS(context->ns);
    context->ns = oldNS;

    XADDRC(Reg::RBP, (int)(INT_SZ * context->ns->size));
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
    LOG_INFO("Registered function #%d with label %d", id, label);
    ByteBufferAppend(&context->funcLabelsBuf, (const char*)&l, sizeof(FuncLable));
    LOG_ASSERT(functionGetL(context, id) == label);
}
unsigned functionGetL(BackendContext* context, int id){
    FuncLable* funcs = (FuncLable*)context->funcLabelsBuf.buff;
    for(size_t i = 0; i * sizeof(FuncLable) < context->funcLabelsBuf.size; ++i){
        if(funcs[i].id == id) return funcs[i].label;
    }
    LOG_ERROR("Function #%d not found", id);
    return 0;
}

void ByteBufferExpand(ByteBuffer* buf){
    buf->buff = (char*)mgk_realloc(buf->buff, buf->capacity *= 2, sizeof(char));
}

int getVarCnt(const Node* node){
    if(node == NULL) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::VAR   ) return 1;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::CALL  ) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::FUNC  ) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::QQ    ) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::TERN_Q) return 0;

    return getVarCnt(node->left) + getVarCnt(node->right);
}

void createFrame(BackendContext* context, NameSpace* par, const Node* node){
    LOG_ASSERT(context != NULL);
    XPUSH(Reg::RBP);
    context->ns = createNS(par);
    int n = getVarCnt(node) * INT_SZ;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::WHILE) n += 8;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::FUNC) n += MIN(6, getNfuncArgs(node->right->left)) * INT_SZ;
    XMOVRR(Reg::RBP, Reg::RSP);
    XSUBRC(Reg::RSP, n);
}

void registerVar(NameSpace* frame, idt_t id, int offset){
    LOG_ASSERT(frame != NULL);
    LOG_ASSERT(frame->varTable != NULL);

    if(frame->size >= frame->capacity){
        expandNS(frame, frame->capacity * 2);
    }
    if(offset == 0){
        offset = -(int)(INT_SZ * (++frame->above));
    }

    frame->varTable[frame->size] = {id, offset};
    frame->size++;
}



void functionEntryVars(BackendContext* context, const Node* node, int* n){
    LOG_ASSERT(context != NULL);
    if(node == NULL) return;
    int k = 0;
    if(n == NULL) n = &k;
    if(node->type == NodeType::IDENTIFIER){
        if(*n < 6){
            registerVar(context->ns, node->data.id);
            int off = -(int)(context->ns->above * INT_SZ);
            xMovRMCR(context, Reg::RBP, off, stdcallRegs[*n]);
        }
        else{
            registerVar(context->ns, node->data.id, (INT_SZ * (*n - 6 + 2)));
        }
        (*n)++;
    }
    functionEntryVars(context, node->left , n);
    functionEntryVars(context, node->right, n);
}

void closeFrame(BackendContext* context){
    LOG_ASSERT(context != NULL);
    NameSpace* oldNS = context->ns->parent;
    deleteNS(context->ns);
    context->ns = oldNS;

    XMOVRR(Reg::RSP, Reg::RBP);
    XPOP(Reg::RBP);    
}