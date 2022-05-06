#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "Backend.h"
#include "Elfer.h"
#include <stdarg.h>
#include <string.h>
const size_t INIT_SZ = 32;

const int INT_SZ = 8; //sizeof(int);


#define ASM(format, ...) {ByteBufferAppendf(&context->asmBuf, format "\n\t", ## __VA_ARGS__);}
#define UPDATE_ADDR(of) *(int*)(&context->binBuf.buff[of - 4]) = (int)(context->binBuf.size - of)

const Reg stdcallRegs[6] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};

const size_t regStackSz = 14;
const Reg regStack[14]  = {Reg::RAX, Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9, Reg::RBX, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

NameSpace* createNS(NameSpace* parent = NULL){
    NameSpace* ns = (NameSpace*)mgk_calloc(1, sizeof(NameSpace));
    ns->parent    = parent;
    ns->size      = 0;
    ns->varTable  = (VarOffset*)mgk_calloc(32, sizeof(VarOffset));
    ns->pushed    = 0;
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
        ".global _start\n"
        "_start:\n");
    
    LabelReserve(&context, 1);  // Take label 0;
    createFrame(&context, NULL, root);
    codeGen(&context, root);
    closeFrame(&context);
    xExit(&context);

    deleteNS(context.ns);
    char* asmFilename = (char*)calloc(strlen(filename) + 5, sizeof(char));
    strcpy(asmFilename, filename);
    strcat(asmFilename, ".s");
    FILE* asmFile = fopen(asmFilename, "w");
    free(asmFilename);
    LOG_ASSERT(asmFile != NULL);
    fwrite(context.asmBuf.buff, sizeof(char), context.asmBuf.size, asmFile);

    genElf(filename, context.binBuf.buff, context.binBuf.size);

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
    LOG_INFO("Offset: 8 * (%d + %d) + %d", (int)(ns->parent->above), (int)ns->parent->pushed, getOffset(ns->parent, id));
    return (int)((ns->parent->above + ns->parent->pushed) * INT_SZ) + getOffset(ns->parent, id);
}

void expandNS(NameSpace* ns, size_t newSZ){
    LOG_ASSERT(ns != NULL);
    if(newSZ <= ns->size) return;

    ns->varTable = (VarOffset*)mgk_realloc(ns->varTable, newSZ, sizeof(VarOffset));

    ns->capacity = newSZ;
}

void codeGen(BackendContext* context, const Node* node){
    LOG_ASSERT(context != NULL);
    context->nTabs++;

    if(node == NULL){
        LOG_ERROR("NULL NODE");
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

    unsigned lbl = 0;
    int pushed = 0;

    if(context->regStackUsed > 10){
        for(size_t i = 0; i < context->regStackUsed; ++i){
            pushed++;
            XPUSH(regStack[i]);
        }
        context->regStackUsed = 0;
    }

    Reg r1 = regStack[context->regStackUsed];
    Reg r2 = regStack[context->regStackUsed + 1];
    Reg r3 = regStack[context->regStackUsed + 2];
    switch (node->type)
    {
    case NodeType::OPERATOR:
    {
        switch (node->data.opr)
        {
    #define CASE_BINARY(opr, OPR)           \
        case Operator::opr:                 \
            codeGen(context, node->left);   \
            codeGen(context, node->right);  \
            OPR(r1, r2);                    \
            context->regStackUsed--;        \
            break


    #define CASE_CMP(opr, flags)            \
        case Operator::opr:                 \
            context->regStackUsed++;        \
            codeGen(context, node->left);   \
            codeGen(context, node->right);  \
            XXOR(r1, r1);                   \
            XCMPRR(r2, r3);                 \
            XSET(flags, r1);                \
            context->regStackUsed -= 2;     \
            break

        CASE_BINARY(ADD, XADD);
        CASE_BINARY(SUB, XSUB);
        CASE_BINARY(MUL, XMUL);
        case Operator::SHL:
            codeGen(context, node->left);   
            codeGen(context, node->right);
            if(r1 == Reg::RCX){
                XMOVRR(r3, r1);
                XMOVRR(r1, r2);
                XSHL(r3, Reg::RCX);
                XMOVRR(r1, r3);                                          
            }
            else{
                XPUSH(Reg::RCX);
                XMOVRR(Reg::RCX, r2);                                                                       
                XSHL(r1, Reg::RCX);   
                XPOP(Reg::RCX);
            }
            context->regStackUsed--;
            break;                                                       
        case Operator::SHR:
            codeGen(context, node->left);   
            codeGen(context, node->right);
            if(r1 == Reg::RCX){
                XMOVRR(r3, r1);
                XMOVRR(r1, r2);
                XSHR(r3, Reg::RCX);
                XMOVRR(r1, r3);                                          
            }
            else{
                XPUSH(Reg::RCX);
                XMOVRR(Reg::RCX, r2);                                                                       
                XSHR(r1, Reg::RCX);   
                XPOP(Reg::RCX);
            }
            context->regStackUsed--;
            break;        
        case Operator::DIV:
        case Operator::MOD:            
            codeGen(context, node->left);
            codeGen(context, node->right);
            if(r2 == Reg::RDX){
                XMOVRR(r3, r2);
            }
            if(r1 != Reg::RDX) XPUSH(Reg::RDX);
            if(r1 != Reg::RAX){
                XPUSH(Reg::RAX);
                XMOVRR(Reg::RAX, r1);
            }
            XCQO();
            XDIV(r2 != Reg::RDX ? r2 : r3);
            if(r1 != Reg::RAX){
                XMOVRR(r1, node->data.opr == Operator::DIV ? Reg::RAX : Reg::RDX);
                XPOP(Reg::RAX);
            }
            if(r1 != Reg::RDX) XPOP(Reg::RDX);
            context->regStackUsed--;
            break;

        CASE_CMP(LESS, Flags::L);
        CASE_CMP(LESS_EQ, Flags::LE);
        CASE_CMP(GRTR, Flags::G);
        CASE_CMP(GRTR_EQ, Flags::GE);
        CASE_CMP(EQUAL, Flags::E);
        CASE_CMP(NON_EQ, Flags::NE);

        

        CASE_BINARY(AND, XAND);
        CASE_BINARY(OR , X_OR);
        CASE_BINARY(XOR, XXOR);
    
    #undef CASE_BINARY
    #undef CASE_CMP
    
        case Operator::SET:
            LOG_ASSERT(node->left != NULL);
            if(node->left->type == NodeType::IDENTIFIER){
                codeGen(context, node->right);
                XMOVVR(getOffset(context->ns, node->left->data.id), r1);
            }
            else{
                LOG_ASSERT(node->left->type == NodeType::OPERATOR);
                LOG_ASSERT(node->left->data.opr == Operator::VAL);
                codeGen(context, node->left->right);
                codeGen(context, node->right);
                XMOVRMR(r1, r2);
                context->regStackUsed--;
            }
            break;
        case Operator::TERN_Q:{
            LOG_ASSERT(node->right->data.opr == Operator::TERN_C);
            codeGen(context, node->left);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            lbl = LabelReserve(context, 2);

            XJMP(Flags::Z, lbl);
            size_t of = context->binBuf.size;
            
            createFrame(context, context->ns, node);
            codeGen(context, node->right->left);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);

            XJMP(Flags::ABS, lbl + 1);
            LabelRegister(context, lbl);
            UPDATE_ADDR(of);
            of = context->binBuf.size;
            
            createFrame(context, context->ns, node);
            codeGen(context, node->right->right);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);
            

            LabelRegister(context, lbl + 1);
            UPDATE_ADDR(of);
            context->regStackUsed++;
        }
            break;
        case Operator::QQ:{
            codeGen(context, node->left);
            XPOP(r1);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            lbl = LabelReserve(context, 1);
            XJMP(Flags::NZ, lbl);
            size_t of = context->binBuf.size;
            createFrame(context, context->ns, node);
            codeGen(context, node->right);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);
            LabelRegister(context, lbl);
            UPDATE_ADDR(of);
            context->regStackUsed++;
        }
            break;
        case Operator::ENDL:
            codeGen(context, node->left);
            if(node->right){
                context->regStackUsed--;
                LOG_INFO("%u", context->regStackUsed);
                LOG_ASSERT(context->regStackUsed == 0);
                codeGen(context, node->right);
            }
            break;
        case Operator::VAR:
            LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);
            registerVar(context->ns, node->left->data.id);
            codeGen(context, node->right);
            XMOVVR(getOffset(context->ns,node->left->data.id), r1);
            break;
        case Operator::FUNC:{
            
            lbl = LabelReserve(context, 2);
            functionRegister(context, node->left->data.id, lbl, 0);
            XJMP(Flags::ABS, lbl + 1);
            LabelRegister(context, lbl);
            size_t of1 = context->binBuf.size;
            NameSpace* oldNS = context->ns;

            createFrame(context, NULL, node);
            functionEntryVars(context, node->right->left, NULL);

            codeGen(context, node->right->right);
            closeFrame(context);
            context->ns = oldNS;
            context->ns->pushed--;
            XRET();

            LabelRegister(context, lbl + 1);
            UPDATE_ADDR(of1);
            XXOR(r1, r1);
            context->regStackUsed++;

            }
            break;
        case Operator::CALL:
            {
            for(size_t i = 0; i < context->regStackUsed; ++i){
                XPUSH(regStack[i]);
            }
            unsigned savedReg = context->regStackUsed;
            context->regStackUsed = 0;

            int n = evaluteArguments(context, node->right);
            for(int i = 0; i < MIN(n, 6); ++i){
                XPOP(stdcallRegs[i]);
            }
            XCALL(functionGetL(context, node->left->data.id));
            XMOVRR(r1, Reg::RAX);
            if(n > 6) XADDRC(Reg::RSP, INT_SZ * (n - 6));
            context->regStackUsed = savedReg + 1;
            while (savedReg)
            {
                XPOP(regStack[savedReg--]);
            }
            }
            break;
        case Operator::WHILE:{
            lbl = LabelReserve(context, 2);
            createFrame(context, context->ns, node);            
            LabelRegister(context, lbl);
            codeGen(context, node->left);
            XTESTRR(Reg::RAX, Reg::RAX);
            context->regStackUsed--;

            XJMP(Flags::Z  , lbl + 1);
            size_t of = context->binBuf.size;
            codeGen(context, node->right);            
            XJMP(Flags::ABS, lbl);
            LabelRegister(context, lbl + 1);
            closeFrame(context);
            UPDATE_ADDR(of);
            XMOVRR(r1, Reg::RAX);
            
        }
            break;
        case Operator::LAND:{
            size_t ofs[3] = {};
            lbl = LabelReserve(context, 2);
            codeGen(context, node->left);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            XJMP(Flags::Z, lbl);
            ofs[0] = context->binBuf.size;

            codeGen(context, node->right);
            context->regStackUsed--;
            XTESTRR(r1, r1);
            XJMP(Flags::Z, lbl);
            ofs[1] = context->binBuf.size;

            XMOVRC(r1, 1);
            XJMP(Flags::ABS, lbl + 1);
            ofs[2] = context->binBuf.size;

            LabelRegister(context, lbl);
            UPDATE_ADDR(ofs[0]);
            UPDATE_ADDR(ofs[1]);

            XXOR(r1, r1)
            context->regStackUsed++;
            LabelRegister(context, lbl + 1);
            UPDATE_ADDR(ofs[2]);
        }
            break;
        case Operator::LOR:{
            size_t ofs[3];

            lbl = LabelReserve(context, 2);
            codeGen(context, node->left);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            XJMP(Flags::NZ, lbl);
            ofs[0] = context->binBuf.size;

            codeGen(context, node->right);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            XJMP(Flags::NZ, lbl);
            ofs[1] = context->binBuf.size;
           
            XMOVRC(r1, 0);
            XJMP(Flags::ABS, lbl + 1);
            ofs[2] = context->binBuf.size;


            LabelRegister(context, lbl);
            UPDATE_ADDR(ofs[0]);
            UPDATE_ADDR(ofs[1]);

            XMOVRC(r1, 1);
            context->regStackUsed++;
            LabelRegister(context, lbl + 1);
            UPDATE_ADDR(ofs[2]);
        }
            break;
        case Operator::NOT:
            codeGen(context, node->right);
            XTESTRR(r1, r1);
            XMOVRC(r1, 0);
            XSET(Flags::Z, r1);
            break;
        case Operator::INC:
            if(node->left){
                LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);

                XMOVRV(r2, getOffset(context->ns, node->left->data.id));
                XMOVRR(r1, r2);
                XINC(r2);
                XMOVVR(getOffset(context->ns, node->left->data.id), r2);
                context->regStackUsed++;
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(r1, getOffset(context->ns, node->right->data.id));
                XINC(r1);
                XMOVVR(getOffset(context->ns, node->right->data.id), r1);
                context->regStackUsed++;
            }
            break;
        case Operator::DEC:
            if(node->left){
                LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);

                XMOVRV(r2, getOffset(context->ns, node->left->data.id));
                XMOVRR(r1, r2);
                XDEC(r2);
                XMOVVR(getOffset(context->ns, node->left->data.id), r2);
                context->regStackUsed++;
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(r1, getOffset(context->ns, node->right->data.id));
                XDEC(r1);
                XMOVVR(getOffset(context->ns, node->right->data.id), r1);
                context->regStackUsed++;
            }
            break;
            break;
        case Operator::ADDR:
            XLEARV(r1, getOffset(context->ns, node->right->data.id));
            context->regStackUsed++;
            break;
        case Operator::VAL:
            codeGen(context, node->right);
            XMOVRRM(r1, r1);
            break;
        case Operator::IN:
            if(r1 != Reg::RAX) XPUSH(Reg::RAX);
            XIN();
            XMOVRR(r1, Reg::RAX);
            if(r1 != Reg::RAX) XPOP(Reg::RAX);
            context->regStackUsed++;
            break;
        case Operator::OUT:
            codeGen(context, node->right);
            if(r1 != Reg::RDI) XPUSH(Reg::RDI);
            if(r1 != Reg::RAX) XPUSH(Reg::RAX);
            XMOVRR(Reg::RDI, r1);
            XOUT();
            XMOVRR(r1, Reg::RAX);
            if(r1 != Reg::RAX) XPOP(Reg::RAX);
            if(r1 != Reg::RDI) XPOP(Reg::RDI);
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
        XMOVRC(r1, node->data.num);
        context->regStackUsed++;
        break;
    case NodeType::IDENTIFIER:
        XMOVRV(r1, getOffset(context->ns, node->data.id));
        context->regStackUsed++;
        break;
    case NodeType::CUSTOM:
    case NodeType::NONE:
    default:
        LOG_ERROR("Incorrect tree!");
        abort();
        break;
    }

    if(pushed){
        XMOVRR(regStack[pushed], r1);
        while (pushed)
        {
            XPOP(regStack[--pushed]);
        }
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
        XPUSH(regStack[--context->regStackUsed]);
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
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::FUNC) n += MIN(6, getNfuncArgs(node->right->left)) * INT_SZ;
    XMOVRR(Reg::RBP, Reg::RSP);

    if(par){

        for(size_t i = 1; i < context->regStackUsed; ++i){
            xPushR(context, regStack[i]);
        }
        context->ns->regSaved = context->regStackUsed;
        context->regStackUsed = 0;
    }
    
    XSUBRC(Reg::RSP, n);
}

void registerVar(NameSpace* frame, idt_t id, int offset){
    LOG_ASSERT(frame != NULL);
    LOG_ASSERT(frame->varTable != NULL);

    if(frame->size >= frame->capacity){
        expandNS(frame, frame->capacity * 2);
    }
    if(offset == 0){
        offset = -(int)(INT_SZ * (++frame->above + frame->regSaved));
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
    if(oldNS && context->ns->regSaved){

        context->regStackUsed = context->ns->regSaved;
        while(--context->ns->regSaved){     //Skips Reg::Rax. As it is returned value
            xPopR(context, regStack[context->ns->regSaved]);
        }
    } else context->regStackUsed = 0;
    deleteNS(context->ns);
    context->ns = oldNS;

    XMOVRR(Reg::RSP, Reg::RBP);
    XPOP(Reg::RBP);    
}