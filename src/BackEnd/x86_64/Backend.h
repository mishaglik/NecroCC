#ifndef NECROCC_BACKEND_X86_64_BACKEND_H
#define NECROCC_BACKEND_X86_64_BACKEND_H
#include "../../LangTree/Tree.h"

struct VarOffset
{
    idt_t id   = 0;
    int offset = 0;
};

struct FuncLable
{
    idt_t id  = 0;
    unsigned label = 0;
    int nArgs = 0;
};

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

const size_t NRegs = 13;

enum class Reg{
    NONE  = -1,
    RAX   = 0,
    RCX   = 1,
    RDX   = 2,
    RBX   = 3,
    RSP   = 4,
    RBP   = 5,
    RSI   = 6,
    RDI   = 7,
    R8    = 8,
    R9    = 9,
    R10   = 10,
    R11   = 11,
    R12   = 12,
    R13   = 13,
    R14   = 14,
    R15   = 15,
};

/**
 * @brief flag & 0x0f - gives binary ref to in.
 *        flag & 0xf0 - just counter not to collide.
 * 
 */
enum class Flags{
    ABS = 0xff,
    
    O   = 0x00,
    
    NO  = 0x01,

    B   = 0x12,
    C   = 0x22,
    NAE = 0x32,

    NB  = 0x03,
    NC  = 0x13,
    AE  = 0x23,

    Z   = 0x04,
    E   = 0x14,

    NZ  = 0x05,
    NE  = 0x15,

    BE  = 0x06,
    NA  = 0x16,

    NBE = 0x07,
    A   = 0x17,

    S   = 0x08,

    NS  = 0x09,

    P   = 0x0a,
    PE  = 0x1a,

    NP  = 0x0b,
    PO  = 0x1b,

    L   = 0x0c,
    NGE = 0x1c,

    NL  = 0x0d,
    GE  = 0x1d,

    LE  = 0x0e,
    NG  = 0x1e,

    NLE = 0x0f,
    G   = 0x1f,
};

struct NameSpace
{
    NameSpace* parent   = NULL;
    VarOffset* varTable = NULL;
    

    size_t size     = 0;
    size_t capacity = 0;
    size_t above    = 0;
    size_t pushed   = 0;
};

struct BackendContext
{
    NameSpace* ns = NULL;
    ByteBuffer asmBuf;
    ByteBuffer binBuf;
    ByteBuffer labelBuf;
    ByteBuffer funcLabelsBuf;
    int pass = 0;
    unsigned labels = 0;
    int nTabs = 0;
};



unsigned LabelReserve(BackendContext* context, unsigned n);
void LabelRegister(BackendContext* context, unsigned label);
long long LabelGetOffs(BackendContext* context, unsigned label);

void functionRegister(BackendContext* context, int id, unsigned label, int nargs);
unsigned functionGetL(BackendContext* context, int id);


void VariableSet(BackendContext* context, Reg r, int offset);

NameSpace* createNS(NameSpace* parent);

void deleteNS(NameSpace* ns);

void backend(const Node* root, const char* filename);

int getOffset(NameSpace* ns, idt_t id);

void expandNS(NameSpace* ns, size_t newSZ);

void registerVar(NameSpace* ns, idt_t id, int offset = 0);

void codeGen(BackendContext* context, const Node* node);

void regVars(NameSpace* ns, const Node* node);

int getNfuncArgs(const Node* node);

int evaluteArguments(BackendContext* context, const Node* node);

void openNewNS(BackendContext* context);

void closeNS(BackendContext* context);

void createFrame(BackendContext* context, int par = 0);

int getVarCnt(const Node* node);

void createFrame(BackendContext* context, NameSpace* par, const Node* node);

void closeFrame(BackendContext* context);

void functionEntryVars(BackendContext* context, const Node* node, int* n = NULL);

#include "Operations.h"

#endif