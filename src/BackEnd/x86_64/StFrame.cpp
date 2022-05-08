#include "MGK/Logger.h"
#include "MGK/Utils.h"
#include "StFrame.h"
#include <string.h>
#include <stdarg.h>
extern const size_t INT_SZ;

Nameframe* createNF(Nameframe* parent = NULL){
    Nameframe* nf = (Nameframe*)mgk_calloc(1, sizeof(Nameframe));
    nf->parent    = parent;
    nf->size      = 0;
    nf->varTable  = (VarOffset*)mgk_calloc(32, sizeof(VarOffset));
    nf->pushed    = 0;
    return nf; 
}

void deleteNF(Nameframe* nf){
    if(!nf) return;
    free(nf->varTable);
    nf->capacity = 0;
    free(nf);
}

int getOffset(Nameframe* nf, idt_t id){
    LOG_ASSERT(nf != NULL);
    LOG_ASSERT(nf->varTable != NULL);

    for(size_t i = 0; i < nf->size; ++i){
        if(nf->varTable[i].id == id){
            return nf->varTable[i].offset;
        }
    }
    if(nf->parent == NULL){
        LOG_ERROR("");
        LOG_STYLE(ConsoleStyle::RED);
        LOG_ERROR("\bVAR %d not registered", id);
        return -1;
    }
    int off = getOffset(nf->parent, id);
    LOG_INFO("Offset: (%d + 8 * %d) + %d", (int)(nf->parent->nSub), (int)nf->parent->pushed, off);
    return (int)(nf->parent->nSub + (nf->parent->pushed) * INT_SZ) + off;
}


void expandNS(Nameframe* nf, size_t newSZ){
    LOG_ASSERT(nf != NULL);
    if(newSZ <= nf->size) return;

    nf->varTable = (VarOffset*)mgk_realloc(nf->varTable, newSZ, sizeof(VarOffset));

    nf->capacity = newSZ;
}

void regVars(Nameframe* nf, const Node* node){
    if(node == NULL){
        return;
    }
    if(node->type == NodeType::IDENTIFIER){
        registerVar(nf, node->data.id);
        return;
    }
    if(node->type == NodeType::OPERATOR && node->data.opr == Operator::COMMA){
        regVars(nf, node->left);
        regVars(nf, node->right);
        return;
    }
    LOG_ERROR("Incorrect expr tree");
}

void registerVar(Nameframe* frame, idt_t id, int offset){
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

label_t LabelReserve(LabelBuf* lbuf, unsigned n){
    LOG_ASSERT(lbuf != NULL);
    label_t l = lbuf->labels;
    lbuf->labels += n;
    while(lbuf->buff.capacity < lbuf->labels * sizeof(size_t)){
        ByteBufferExpand(&lbuf->buff);
    }
    return l;
}

void LabelRegister(LabelBuf* lbuf, label_t label, size_t lVal){
    LOG_ASSERT(lbuf != NULL);
    LOG_ASSERT(label < lbuf->labels);
    ((size_t*)lbuf->buff.buff)[label] = lVal;
}

long long LabelGetOffs(const LabelBuf* lbuf, label_t label){
    LOG_ASSERT(lbuf != NULL);
    LOG_ASSERT(label < lbuf->labels);

    return ((long long*) lbuf->buff.buff)[label];
}

void functionRegister(ByteBuffer* funcLabelsBuf, int id, label_t label, int nargs){
    FuncLable l = {.id = id, .label = label, .nArgs = nargs};
    LOG_INFO("Registered function #%d with label %d", id, label);
    ByteBufferAppend(funcLabelsBuf, (const char*)&l, sizeof(FuncLable));
    LOG_ASSERT(functionGetL(funcLabelsBuf, id) == label);
}

label_t functionGetL(const ByteBuffer* funcLabelsBuf, int id){
    const FuncLable* funcs = (const FuncLable*)funcLabelsBuf->buff;
    for(size_t i = 0; i * sizeof(FuncLable) < funcLabelsBuf->size; ++i){
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