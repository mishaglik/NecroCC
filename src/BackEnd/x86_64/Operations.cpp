#include "Backend.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"


#define ASM(format, ...) {ByteBufferAppendf(&context->asmBuf, format "\n\t", ## __VA_ARGS__);}
#define BIN(buf, sz) ByteBufferAppend(&context->binBuf, (const char*)buf, sz)

#define PUT_ARG(place, x) *((long long*)&(place)) = (x)
#define REG3BIT(r)  (unsigned char)((unsigned char)(r) & 0x7)
#define REGEXTR(r)  (unsigned char)((unsigned char)(r) >> 3)
#define FLAFBIT(f)  (unsigned char)((unsigned char)(f) & 0xf)
#define UC (unsigned char)

void xCqo(BackendContext* context){
    LOG_ASSERT(context != NULL);
    ASM("cqo");
     unsigned char buf[2] = {0x48, 0x99};
    BIN(buf, 2);
}

void xPushR(BackendContext* context, Reg r){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[2];

    ASM("push %s", getRegName(r));

    if(r < Reg::R8){
        buf[0] = UC (0x50 | REG3BIT(r));
        BIN(buf, 1);
    }
    else{
        buf[0] = UC 0x41;
        buf[1] = UC (0x50 | REG3BIT(r));
        BIN(buf, 2);
    }
}

void xPopR(BackendContext* context, Reg r){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[2];

    ASM("pop %s", getRegName(r));

    if(r < Reg::R8){
        buf[0] = UC (0x58 | REG3BIT(r));
        BIN(buf, 1);
    }
    else{
        buf[0] = UC 0x41;
        buf[1] = UC (0x58 | REG3BIT(r));
        BIN(buf, 2);
    }
}

void xPushMRC(BackendContext* context, Reg r, long long c){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[7];

    ASM("push [%s %c %#llx]", getRegName(r), (c >= 0 ? '+' : '-'), std::abs(c));

    if(!REGEXTR(r)){
        buf[0] = UC 0xff;
        buf[1] = UC (0xb0 | REG3BIT(r));
        PUT_ARG(buf[2], c);
        BIN(buf, 6);
    }
    else{
        buf[0] = UC 0x41;
        buf[1] = UC 0xff;
        buf[2] = UC (0xb0 | REG3BIT(r));
        PUT_ARG(buf[3], c);
        BIN(buf, 7);
    }
}

void xPopMRC(BackendContext* context, Reg r, long long c){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[7];

    ASM("pop [%s %c %#llx]", getRegName(r), (c >= 0 ? '+' : '-'), std::abs(c));

    if(!REGEXTR(r)){
        buf[0] = UC 0x8f;
        buf[1] = UC 0x80 | REG3BIT(r);
        PUT_ARG(buf[2], c);
        BIN(buf, 6);
    }
    else{
        buf[0] = UC 0x41;
        buf[1] = UC 0x8f;
        buf[2] = UC 0x80 | REG3BIT(r);
        PUT_ARG(buf[3], c);
        BIN(buf, 7);
    }
}

void xCmpRR(BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r1 != Reg::NONE);
    LOG_ASSERT(r2 != Reg::NONE);
     unsigned char buf[3] = {0x48, 0x39, 0xc0};
    ASM("cmp %s, %s", getRegName(r1), getRegName(r2));

    buf[3] |= REG3BIT(r1);
    buf[3] |= REG3BIT(r2) << 3;
    buf[0] |= REGEXTR(r1);
    buf[0] |= REGEXTR(r2) << 2;

    BIN(buf, 3);
}

void xCmpRC(BackendContext* context, Reg r, long long c){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
    LOG_ASSERT(r < Reg::RSP || r > Reg::RDI);

     unsigned char buf[7] = {0x48, 0x81, 0xf8};
    ASM("cmp %s, %lld", getRegName(r), c);

    if(r == Reg::RAX){
        buf[2] = 0x3d;
        PUT_ARG(buf[3], c);
        BIN(buf, 6);
        return;
    }

    buf[0] |= REGEXTR(r);
    buf[3] |= REG3BIT(r);
    PUT_ARG(buf[4], c);
    BIN(buf, 7);
}

void xExit(BackendContext* context){
    LOG_ASSERT(context != NULL);
    ASM("xor rdi, rdi");
    ASM("mov rax, 0x3c");
    ASM("syscall");
    const  unsigned char buf[12] = {
        0x48, 0x31, 0xff,   // xor rdi, rdi 
        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,  // mov rax, 0x3c (exit code);
        0x0f, 0x05  // syscall
    };
    BIN(buf, 12);
}

void xIn(BackendContext* context){
    ASM("push 0; in")
    const  unsigned char buf[2] = {0x6a, 0x00};
    BIN(buf, 2);
}
void xOut(BackendContext* context){
    ASM("mov rax, rdi; out")
    const  unsigned char buf[3] = {0x48, 0x89, 0xf8};
    BIN(buf, 3);
}
void xOutC(BackendContext* context){
    ASM("mov rax, rdi; out")
    const  unsigned char buf[3] = {0x48, 0x89, 0xf8};
    BIN(buf, 3);
}

void xLeaRMRC(BackendContext* context, Reg r1, Reg r2, long long c){
    LOG_ASSERT(context != NULL);
    ASM("lea %s, [%s + %#llx]", getRegName(r1), getRegName(r2), (unsigned long long)c);
     unsigned char buf[8] = {0x48, 0x8d, 0x80};

    buf[0] |= REGEXTR(r1);
    buf[0] |= REGEXTR(r2) << 2;
    buf[3] |= REG3BIT(r2);
    buf[3] |= REG3BIT(r1) << 3;

    if(r2 == Reg::RSP){
        buf[4] = 0x24;
        PUT_ARG(buf[5], c);
        BIN(buf, 8);
        return;
    }
    PUT_ARG(buf[4], c);
    BIN(buf, 7);
}

void xRet(BackendContext* context){
    LOG_ASSERT(context != NULL);
    ASM("ret")
     unsigned char buf[1] = {0xc3};
    BIN(buf, 1);
}

void xSetR(BackendContext* context, Flags f, Reg r){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(f != Flags::ABS);
    LOG_ASSERT(r != Reg::NONE);

    ASM("set%s %s", getFlagName(f), getReg8Name(r));

     unsigned char buf[4];
    size_t i = 0;
    if(r >= Reg::RSP){
        buf[0] = 0x40;
        buf[0] |= REGEXTR(r); 
        i = 1;
    }

    buf[i++] = 0x0f; 
    buf[i++] = 0x90 | FLAFBIT(f);
    buf[i++] = 0xc0 | REG3BIT(r);
    BIN(buf, i); 
}

void xCallC(BackendContext* context, unsigned l){
    LOG_ASSERT(context != NULL);
    ASM("call L%u", l);
    long long afterIp = context->binBuf.size + 5;
    unsigned char buf[5] = {0xe8};
    PUT_ARG(buf[1], LabelGetOffs(context, l) - afterIp);
    BIN(buf, 5);
}

void xJmpC(BackendContext* context, Flags f, unsigned l){
    LOG_ASSERT(context != NULL);
    if(f == Flags::ABS){
        ASM("jmp L%u", l);
    } else ASM("j%s L%u", getFlagName(f), l);

    long long afterIp = context->binBuf.size + 5;
    unsigned char buf[6] = {};
    if(f == Flags::ABS){
        buf[0] = 0xe9;
        PUT_ARG(buf[1], LabelGetOffs(context, l) - afterIp);
        BIN(buf, 5);
        return;
    }
    buf[0] = 0x0f;
    buf[1] = 0x80 | FLAFBIT(f);
    PUT_ARG(buf[2], LabelGetOffs(context, l) - afterIp);
    BIN(buf, 6);
}

void xTestRR(BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r1 != Reg::NONE);
    LOG_ASSERT(r2 != Reg::NONE);
     unsigned char buf[3] = {0x48, 0x85, 0xc0};
    ASM("test %s, %s", getRegName(r1), getRegName(r2));

    buf[3] |= REG3BIT(r1);
    buf[3] |= UC (REG3BIT(r2) << 3);
    buf[0] |= REGEXTR(r1);
    buf[0] |= UC (REGEXTR(r2) << 2);

    BIN(buf, 3);
}

void xIncR(BackendContext* context, Reg r){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[3] = {0x48, 0xff, 0xc0};
    ASM("inc %s", getRegName(r));

    buf[0] |= REGEXTR(r);
    buf[3] |= REG3BIT(r);

    BIN(buf, 3);
}
void xDecR(BackendContext* context, Reg r){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r != Reg::NONE);
     unsigned char buf[3] = {0x48, 0xff, 0xc8};
    ASM("dec %s", getRegName(r));

    buf[0] |= REGEXTR(r);
    buf[3] |= REG3BIT(r);

    BIN(buf, 3);
}

void xMovRR  (BackendContext* context, Reg r1, Reg r2){
    ASM("mov %s, %s", getRegName(r1), getRegName(r2));
    xBinary(context, 0x89, r1, r2);
}

void xMovRRM (BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    ASM("mov %s, [%s]", getRegName(r1), getRegName(r2));
     unsigned char buf[4] = {0x48, 0x8b, 0x00};
    buf[0] |= UC (REGEXTR(r1) << 2);
    buf[0] |= REGEXTR(r2);

    buf[3] |= UC (REG3BIT(r1) << 3);
    buf[3] |= REG3BIT(r2);

    if(r2 == Reg::RBP){
        buf[3] = UC (0x45 | (REG3BIT(r1) << 3));
        buf[4] = 0x00;
        BIN(buf, 4);
        return;
    }
    if(r2 == Reg::RSP){
        buf[3] = UC (0x04 | (REG3BIT(r1) << 3));
        buf[4] = 0x24;
        BIN(buf, 4);
        return;
    }
    BIN(buf, 3);
}
void xMovRMR (BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    ASM("mov [%s], %s", getRegName(r1), getRegName(r2));

     unsigned char buf[4] = {0x48, 0x89, 0x00};
    buf[0] |= REGEXTR(r1);
    buf[0] |= UC (REGEXTR(r2) << 2);

    buf[3] |= REG3BIT(r1);
    buf[3] |= UC (REG3BIT(r2) << 3);

    if(r1 == Reg::RBP){
        buf[3] = UC (0x45 | (REG3BIT(r2) << 3));
        buf[4] = 0x00;
        BIN(buf, 4);
        return;
    }
    if(r1 == Reg::RSP){
        buf[3] = UC (0x04 | (REG3BIT(r2) << 3));
        buf[4] = 0x24;
        BIN(buf, 4);
        return;
    }
    BIN(buf, 3);
}

void xMovRRMC(BackendContext* context, Reg r1, Reg r2, long long off){
    LOG_ASSERT(context != NULL);
    ASM("mov %s, [%s + %#x]", getRegName(r1), getRegName(r2), (unsigned long long) off);

     unsigned char buf[8] = {0x48, 0x8b, 0x80};
    buf[0] |= UC (REGEXTR(r1) << 2);
    buf[0] |= REGEXTR(r2);

    buf[3] |= UC (REG3BIT(r1) << 3);
    buf[3] |= REG3BIT(r2);

    if(r2 == Reg::RSP){
        buf[3] = UC (0x84 | (REG3BIT(r1) << 3));
        buf[4] = 0x24;
        PUT_ARG(buf[5], off);
        BIN(buf, 8);
        return;
    }
    PUT_ARG(buf[4], off);
    BIN(buf, 7);
}
void xMovRMCR(BackendContext* context, Reg r1, long long off, Reg r2){
    LOG_ASSERT(context != NULL);
    ASM("mov [%s + %#x], %s", getRegName(r1), (unsigned long long) off, getRegName(r2));

     unsigned char buf[4] = {0x48, 0x89, 0x80};
    buf[0] |= REGEXTR(r1);
    buf[0] |= UC (REGEXTR(r2) << 2);

    buf[3] |= REG3BIT(r1);
    buf[3] |= UC (REG3BIT(r2) << 3);

    if(r1 == Reg::RSP){
        buf[3] = UC (0x84 | (REG3BIT(r2) << 3));
        buf[4] = 0x24;
        PUT_ARG(buf[5], off);
        BIN(buf, 8);
        return;
    }
    PUT_ARG(buf[4], off);
    BIN(buf, 7);
}
void xMovRC  (BackendContext* context, Reg r, long long c){
    LOG_ASSERT(context != NULL);
    ASM("mov %s, %lld", getRegName(r), c);
     unsigned char buf[7] = {0x48, 0xc7, 0xc0};
    buf[0] |= REGEXTR(r);
    buf[3] |= REG3BIT(r);
    PUT_ARG(buf[4], c);
    BIN(buf, 7);
}

void xBinary(BackendContext* context, unsigned char opcode, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
     unsigned char buf[3] = {0x48, opcode, 0xc0};
    buf[0] |= REGEXTR(r1);
    buf[0] |= UC (REGEXTR(r2) << 2);

    buf[3] |= REG3BIT(r1);
    buf[3] |= UC (REG3BIT(r2) << 3);

    BIN(buf, 3);
}

void xIMulRR(BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    ASM("imul %s, %s", getRegName(r1), getRegName(r2));
    
     unsigned char buf[4] = {0x48, 0x0f, 0xaf, 0xc0};
    buf[0] |= REGEXTR(r1);
    buf[0] |= UC (REGEXTR(r2) << 2);

    buf[4] |= REG3BIT(r1);
    buf[4] |= UC (REG3BIT(r2) << 3);

    BIN(buf, 4);
}

void xIDivR(BackendContext* context, Reg r){
    LOG_ASSERT(context != NULL);
    ASM("idiv %s", getRegName(r));
    
     unsigned char buf[3] = {0x48, 0xf7, 0xf8};
    buf[0] |= REGEXTR(r);
    buf[3] |= REG3BIT(r);

    BIN(buf, 4);
}

void xShlRR(BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r2 == Reg::RCX);
    ASM("shl %s cl", getRegName(r1));
     unsigned char buf[3] = {0x48, 0xd3, 0xe0};
    buf[0] |= REGEXTR(r1);
    buf[3] |= REG3BIT(r1);
    BIN(buf, 3);
}
void xShrRR(BackendContext* context, Reg r1, Reg r2){
    LOG_ASSERT(context != NULL);
    LOG_ASSERT(r2 == Reg::RCX);
    ASM("shr %s cl", getRegName(r1));
    unsigned char buf[3] = {0x48, 0xd3, 0xe8};
    buf[0] |= REGEXTR(r1);
    buf[3] |= REG3BIT(r1);
    BIN(buf, 3);
}

const char* getRegName(Reg r){
    switch (r)
    {
    case Reg::RAX: return "rax"; 
    case Reg::RCX: return "rcx"; 
    case Reg::RDX: return "rdx"; 
    case Reg::RBX: return "rbx"; 
    case Reg::RSP: return "rsp"; 
    case Reg::RBP: return "rbp"; 
    case Reg::RSI: return "rsi"; 
    case Reg::RDI: return "rdi";
    case Reg::R8 : return "r8" ; 
    case Reg::R9 : return "r9" ; 
    case Reg::R10: return "r10"; 
    case Reg::R11: return "r11"; 
    case Reg::R12: return "r12"; 
    case Reg::R13: return "r13"; 
    case Reg::R14: return "r14"; 
    case Reg::R15: return "r15"; 

    case Reg::NONE: 
    default:
        LOG_ERROR("Wrong register\n");
        break;
    }
    return NULL;
}

const char* getReg8Name(Reg r){
    switch (r)
    {
    case Reg::RAX: return "al"; 
    case Reg::RCX: return "cl"; 
    case Reg::RDX: return "dl"; 
    case Reg::RBX: return "bl"; 
    case Reg::RSP: return "spl"; 
    case Reg::RBP: return "bpl"; 
    case Reg::RSI: return "sil"; 
    case Reg::RDI: return "dil";
    case Reg::R8 : return "r8b"; 
    case Reg::R9 : return "r9b"; 
    case Reg::R10: return "r10b"; 
    case Reg::R11: return "r11b"; 
    case Reg::R12: return "r12b"; 
    case Reg::R13: return "r13b"; 
    case Reg::R14: return "r14b"; 
    case Reg::R15: return "r15b"; 

    case Reg::NONE: 
    default:
        LOG_ERROR("Wrong register\n");
        break;
    }
    return NULL;
}

const char* getFlagName(Flags f){
    switch (f)
    {
    case Flags::O   : return "o";
    case Flags::NO  : return "no";
    case Flags::B   : return "b";
    case Flags::C   : return "c";
    case Flags::NAE : return "nae";
    case Flags::NB  : return "nb";
    case Flags::NC  : return "nc";
    case Flags::AE  : return "ae";
    case Flags::Z   : return "z";
    case Flags::E   : return "e";
    case Flags::NZ  : return "nz";
    case Flags::NE  : return "ne";
    case Flags::BE  : return "be";
    case Flags::NA  : return "na";
    case Flags::NBE : return "nbe";
    case Flags::A   : return "a";
    case Flags::S   : return "s";
    case Flags::NS  : return "ns";
    case Flags::P   : return "p";
    case Flags::PE  : return "pe";
    case Flags::NP  : return "np";
    case Flags::PO  : return "po";
    case Flags::L   : return "l";
    case Flags::NGE : return "nge";
    case Flags::NL  : return "nl";
    case Flags::GE  : return "ge";
    case Flags::LE  : return "le";
    case Flags::NG  : return "ng";
    case Flags::NLE : return "nle";
    case Flags::G   : return "g";
    case Flags::ABS:
    default:
        LOG_ASSERT(0);
        break;
    }
    return NULL;
}