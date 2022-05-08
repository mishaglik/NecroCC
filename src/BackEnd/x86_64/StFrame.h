#ifndef NECROCC_BACKEND_X86_64_STFRAME_H
#define NECROCC_BACKEND_X86_64_STFRAME_H
#include "../../LangTree/Tree.h"

struct VarOffset
{
    idt_t id   = 0;
    int offset = 0;
};

struct Nameframe
{
    Nameframe* parent   = NULL;
    VarOffset* varTable = NULL;
    

    size_t size     = 0;
    size_t capacity = 0;
    size_t above    = 0;
    size_t pushed   = 0;
    size_t nSub     = 0;
    unsigned regSaved = 0;
};

typedef unsigned label_t;


struct FuncLable
{
    idt_t id  = 0;
    unsigned label = 0;
    int nArgs = 0;
};


Nameframe* createNF(Nameframe* parent);

void deleteNF(Nameframe* nf);

int getOffset(Nameframe* nf, idt_t id);

void expandNS(Nameframe* nf, size_t newSZ);

void registerVar(Nameframe* nf, idt_t id, int offset = 0);

void regVars(Nameframe* nf, const Node* node);

struct ByteBuffer
{
    char* buff;
    size_t size;
    size_t capacity;
};

void newByteBuffer(ByteBuffer* buf);
void deleteByteBuffer(ByteBuffer* buffer);
void ByteBufferAppend(ByteBuffer* buf, const char* data, size_t size);
void ByteBufferAppendf(ByteBuffer* buf, const char* format, ...);
void ByteBufferExpand(ByteBuffer* buf);

int getVarCnt(const Node* node);

struct LabelBuf{
    ByteBuffer buff;
    label_t labels   = 0;
};

int getNfuncArgs(const Node* node);



label_t LabelReserve(LabelBuf* lbuf, unsigned n);

void LabelRegister(LabelBuf* lbuf, label_t label, size_t lVal);

long long LabelGetOffs(const LabelBuf* lbuf, label_t label);

void functionRegister(ByteBuffer* funcLabelsBuf, int id, label_t label, int nargs);

label_t functionGetL(const ByteBuffer* funcLabelsBuf, int id);


#endif