#ifndef NECROCC_BACKEND_X86_64_BACKEND_H
#define NECROCC_BACKEND_X86_64_BACKEND_H
#include "../../LangTree/Tree.h"
#include "StFrame.h"







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


struct BackendContext
{
    Nameframe* nf = NULL;
    ByteBuffer asmBuf;
    ByteBuffer binBuf;
    ByteBuffer funcLabelsBuf;
    LabelBuf   labelBuf;
    int pass = 0;
    int nTabs = 0;
    unsigned regStackUsed = 0;
};

void functionEntryVars(BackendContext* context, const Node* node, int* n);

int evaluteArguments(BackendContext* context, const Node* node);

void createFrame(BackendContext* context, Nameframe* par, const Node* node);

void closeFrame(BackendContext* context);

void VariableSet(BackendContext* context, Reg r, int offset);

void backend(const Node* root, const char* filename);

void codeGen(BackendContext* context, const Node* node);

#include "Operations.h"

#endif