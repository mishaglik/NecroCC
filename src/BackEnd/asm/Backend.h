#ifndef NECROCC_BACKEND_H
#define NECROCC_BACKEND_H
#include "../../LangTree/Tree.h"


struct VarOffset
{
    idt_t id   = 0;
    int offset = 0;
};

struct FuncLable
{
    idt_t id  = 0;
    int nArgs = 0;
};


struct NameSpace
{
    NameSpace* parent   = NULL;
    VarOffset* varTable = NULL;

    int curOffset   = 0;
    size_t size     = 0;
    size_t capacity = 0;
};

struct BackendContext
{
    NameSpace* ns = NULL;
    FILE* asmFile = NULL;
    int nTabs = 0;
};

NameSpace* createNS(NameSpace* parent);

void deleteNS(NameSpace* ns);

void backend(const Node* root, const char* filename);

int getOffset(NameSpace* ns, idt_t id);

void expandNS(NameSpace* ns, size_t newSZ);

void registerVar(NameSpace* ns, idt_t id);

void codeGen(BackendContext* context, const Node* node);

void regVars(NameSpace* ns, const Node* node);

int getNargs(const Node* node);

void evaluteArguments(BackendContext* context, const Node* node, int* offset);

void openNewNS(BackendContext* context);

void closeNS(BackendContext* context);
#endif