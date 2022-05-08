#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "Backend.h"
#include "Elfer.h"
#include "StFrame.h"
#include <stdarg.h>
#include <string.h>

const size_t INIT_SZ = 32;

extern const size_t INT_SZ = 8; // Size of target integer type.

#define ASM(format, ...) {ByteBufferAppendf(&context->asmBuf, format "\n\t", ## __VA_ARGS__);}
#define UPDATE_ADDR(of) *(int*)(&context->binBuf.buff[of - 4]) = (int)(context->binBuf.size - of)
#define LABEL_REGISTER(lbl) ASM("L%u:", lbl); LabelRegister(&context->labelBuf, lbl, context->binBuf.size)

const size_t regStackSz    = 14;
const size_t stdcallRegsSz = 6;

const Reg stdcallRegs[stdcallRegsSz] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
const Reg regStack[regStackSz]       = {Reg::RAX, Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9, Reg::RBX, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

void backend(const Node* root, const char* filename){
    LOG_ASSERT(root != NULL);
    LOG_ASSERT(filename != NULL);

    LOG_FILLER("BACKEND START");


    BackendContext context = {};
    newByteBuffer(&context.asmBuf);
    newByteBuffer(&context.binBuf);
    newByteBuffer(&context.labelBuf.buff);
    newByteBuffer(&context.funcLabelsBuf);

    ByteBufferAppendf(&context.asmBuf,
        ".intel_syntax noprefix\n"
        ".global _start\n"
        "_start:\n");
    
    LabelReserve(&context.labelBuf, 1);  // Take label 0;
    createFrame(&context, NULL, root);
    codeGen(&context, root);
    closeFrame(&context);
    xExit(&context);

    deleteNF(context.nf);

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
    deleteByteBuffer(&context.labelBuf.buff);
    deleteByteBuffer(&context.funcLabelsBuf);
    fclose(asmFile);

    LOG_FILLER("BACKEND END");
}


void codeGen(BackendContext* context, const Node* node){
    LOG_ASSERT(context != NULL);
    context->nTabs++;

    if(node == NULL){
        XXOR(regStack[context->regStackUsed],regStack[context->regStackUsed]);
        context->regStackUsed++;
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

    if(context->regStackUsed > regStackSz - 4){
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
                XMOVVR(getOffset(context->nf, node->left->data.id), r1);
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
            lbl = LabelReserve(&context->labelBuf, 2);

            XJMP(Flags::Z, lbl);
            size_t of = context->binBuf.size;
            
            createFrame(context, context->nf, node->right->left);
            codeGen(context, node->right->left);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);

            XJMP(Flags::ABS, lbl + 1);
            LabelRegister(&context->labelBuf, lbl, context->binBuf.size);
            UPDATE_ADDR(of);
            of = context->binBuf.size;
            
            createFrame(context, context->nf, node->right->right);
            codeGen(context, node->right->right);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);
            
            LABEL_REGISTER(lbl + 1);
            UPDATE_ADDR(of);
            context->regStackUsed++;
        }
            break;
        case Operator::QQ:{
            codeGen(context, node->left);
            XPOP(r1);
            XTESTRR(r1, r1);
            context->regStackUsed--;
            lbl = LabelReserve(&context->labelBuf, 1);
            XJMP(Flags::NZ, lbl);
            size_t of = context->binBuf.size;
            createFrame(context, context->nf, node);
            codeGen(context, node->right);
            XMOVRR(r1, Reg::RAX);
            closeFrame(context);
            LABEL_REGISTER(lbl);
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
            registerVar(context->nf, node->left->data.id);
            codeGen(context, node->right);
            XMOVVR(getOffset(context->nf,node->left->data.id), r1);
            break;
        case Operator::FUNC:{
            
            lbl = LabelReserve(&context->labelBuf, 2);
            functionRegister(&context->funcLabelsBuf, node->left->data.id, lbl, 0);
            XJMP(Flags::ABS, lbl + 1);
            LABEL_REGISTER(lbl);
            size_t of1 = context->binBuf.size;
            Nameframe* oldNS = context->nf;

            createFrame(context, NULL, node);
            functionEntryVars(context, node->right->left, NULL);

            codeGen(context, node->right->right);
            closeFrame(context);
            context->nf = oldNS;
            context->nf->pushed--;
            XRET();

            LABEL_REGISTER(lbl + 1);
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
            XCALL(functionGetL(&context->funcLabelsBuf, node->left->data.id));
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
            lbl = LabelReserve(&context->labelBuf, 2);
            createFrame(context, context->nf, node);
            context->regStackUsed++;   
            LABEL_REGISTER(lbl);         
            codeGen(context, node->left);
            XTESTRR(Reg::RDI, Reg::RDI);
            context->regStackUsed -= 2;

            XJMP(Flags::Z  , lbl + 1);
            size_t of = context->binBuf.size;
            codeGen(context, node->right);            
            XJMP(Flags::ABS, lbl);
            LABEL_REGISTER(lbl + 1);
            UPDATE_ADDR(of);
            closeFrame(context);
            XMOVRR(r1, Reg::RAX);
            
        }
            break;
        case Operator::LAND:{
            size_t ofs[3] = {};
            lbl = LabelReserve(&context->labelBuf, 2);
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

            LABEL_REGISTER(lbl);
            UPDATE_ADDR(ofs[0]);
            UPDATE_ADDR(ofs[1]);

            XXOR(r1, r1)
            context->regStackUsed++;
            LABEL_REGISTER(lbl + 1);
            UPDATE_ADDR(ofs[2]);
        }
            break;
        case Operator::LOR:{
            size_t ofs[3];

            lbl = LabelReserve(&context->labelBuf, 2);
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

            LABEL_REGISTER(lbl);
            UPDATE_ADDR(ofs[0]);
            UPDATE_ADDR(ofs[1]);

            XMOVRC(r1, 1);
            context->regStackUsed++;
            LABEL_REGISTER(lbl + 1);
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

                XMOVRV(r2, getOffset(context->nf, node->left->data.id));
                XMOVRR(r1, r2);
                XINC(r2);
                XMOVVR(getOffset(context->nf, node->left->data.id), r2);
                context->regStackUsed++;
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(r1, getOffset(context->nf, node->right->data.id));
                XINC(r1);
                XMOVVR(getOffset(context->nf, node->right->data.id), r1);
                context->regStackUsed++;
            }
            break;
        case Operator::DEC:
            if(node->left){
                LOG_ASSERT(node->left->type == NodeType::IDENTIFIER);

                XMOVRV(r2, getOffset(context->nf, node->left->data.id));
                XMOVRR(r1, r2);
                XDEC(r2);
                XMOVVR(getOffset(context->nf, node->left->data.id), r2);
                context->regStackUsed++;
            }
            else{
                LOG_ASSERT(node->right);
                LOG_ASSERT(node->right->type == NodeType::IDENTIFIER);

                XMOVRV(r1, getOffset(context->nf, node->right->data.id));
                XDEC(r1);
                XMOVVR(getOffset(context->nf, node->right->data.id), r1);
                context->regStackUsed++;
            }
            break;
            break;
        case Operator::ADDR:
            XLEARV(r1, getOffset(context->nf, node->right->data.id));
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
        XMOVRV(r1, getOffset(context->nf, node->data.id));
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



int evaluteArguments(BackendContext* context, const Node* node){
    LOG_ASSERT(context != NULL);
    if(node == NULL) return 0;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::COMMA){
        return
        evaluteArguments(context, node->right) + 
        evaluteArguments(context, node->left);
    }
    else{
        codeGen(context, node);
        XPUSH(regStack[--context->regStackUsed]);
        return 1;
    }
}

int main(int argc, char* argv[]){
    LOG_ASSERT(argc > 2);
    Tree* tree = readTree(argv[1]);

    backend(tree->root, argv[2]);

    nodeListDtor(tree->list);
    free(tree);
}



void createFrame(BackendContext* context, Nameframe* par, const Node* node){
    LOG_ASSERT(context != NULL);
    XPUSH(Reg::RBP);
    context->nf = createNF(par);

    int n = getVarCnt(node) * INT_SZ;
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::FUNC){
        n = getVarCnt(node->right->right) * INT_SZ;
        n += MIN(6, getNfuncArgs(node->right->left)) * INT_SZ;
    }
    XMOVRR(Reg::RBP, Reg::RSP);

    if(par){
        for(size_t i = 1; i < context->regStackUsed; ++i){
            xPushR(context, regStack[i]);
        }
        context->nf->regSaved = context->regStackUsed;
        context->regStackUsed = 0;
    }
    
    context->nf->nSub = n;
    LOG_INFO("Creating frame with nSub = %d", n);
    XSUBRC(Reg::RSP, n);
}


void functionEntryVars(BackendContext* context, const Node* node, int* n){
    LOG_ASSERT(context != NULL);
    if(node == NULL) return;
    int k = 0;
    if(n == NULL) n = &k;
    if(node->type == NodeType::IDENTIFIER){
        if(*n < 6){
            registerVar(context->nf, node->data.id);
            int off = -(int)(context->nf->above * INT_SZ);
            xMovRMCR(context, Reg::RBP, off, stdcallRegs[*n]);
        }
        else{
            registerVar(context->nf, node->data.id, (INT_SZ * (*n - 6 + 2)));
        }
        (*n)++;
    }
    functionEntryVars(context, node->left , n);
    functionEntryVars(context, node->right, n);
}

void closeFrame(BackendContext* context){
    LOG_ASSERT(context != NULL);
    Nameframe* oldNS = context->nf->parent;
    if(oldNS && context->nf->regSaved){

        context->regStackUsed = context->nf->regSaved;
        while(--context->nf->regSaved){     //Skips Reg::Rax. As it is returned value
            xPopR(context, regStack[context->nf->regSaved]);
        }
    } else context->regStackUsed = 0;
    deleteNF(context->nf);
    context->nf = oldNS;
    LOG_INFO("Closing frame");
    XMOVRR(Reg::RSP, Reg::RBP);
    XPOP(Reg::RBP);    
}