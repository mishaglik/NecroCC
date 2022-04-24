#ifndef NECROCC_BACKEND_X86_64_OPERATIONS_H
#define NECROCC_BACKEND_X86_64_OPERATIONS_H

#ifndef NECROCC_BACKEND_X86_64_BACKEND_H
#error  Wrong include scheme.
#endif

#define XADD(r1, r2) {xBinaryRR(context, 0x01, r1, r2); ASM("add %s %s", getRegName(r1), getRegName(r2));}
#define XSUB(r1, r2) {xBinaryRR(context, 0x29, r1, r2); ASM("add %s %s", getRegName(r1), getRegName(r2));}
#define XMUL(r1, r2) xMulRR(context, r1, r2)
#define XSHL(r1, r2) xShlRR(context, r1, r2)
#define XSHR(r1, r2) xShrRR(context, r1, r2)
#define XDIV(r1, r2) xDivRR(context, r1, r2)

#define CQO() xCqo(context);

#define XOR(r1, r2)  {xBinaryRR(context, 0x09, r1, r2);ASM("add %s %s", getRegName(r1), getRegName(r2));}
#define XAND(r1, r2) {xBinaryRR(context, 0x21, r1, r2);ASM("add %s %s", getRegName(r1), getRegName(r2));}
#define XXOR(r1, r2) {xBinaryRR(context, 0x31, r1, r2);ASM("add %s %s", getRegName(r1), getRegName(r2));}

// mov r1, r2
#define XMOVRR(r1, r2) xMovRR(context, r1, r2)
// mov [rbp + off], r
#define XMOVVR(off, r) xMovRMCR(context, Reg::RBP, off, r)

// mov r, [rbp + off]
#define XMOVRV(r, off) xMovRRMC(context, r, Reg::RBP, off)

//mov r1, [r2]
#define XMOVRRM(r1, r2) xMovRRM(context, r1, r2)

//mov [r1], r2
#define XMOVRMR(r1, r2) xMovRMR(context, r1, r2)

// mov r, c
#define XMOVRC(r, c) xMovRC(context, r, c)

#define XINC(r) xIncR(context, r)
#define XDEC(r) xDecR(context, r)

#define XTESTRR(r1, r2) xTestRR(context, r1, r2)

// jmp rip + off
#define XJMP(cond, l) xJmpC(context, cond, l)

#define XCALL(l) xCallC(contex, l)

// set(cond) r; Ex: sete al;
#define XSET(cond, r) xSetR(context, cond, r)

#define XRET() xRet(context)

//lea r, [rbp + offs]
#define XLEARV(r, offs) xLeaRMRC(context, r, Reg::RBP, offs)
#define XPUSH(r) xPushR(context, r)
#define XPOP(r)  xPopR(context, r)

#define XPOPMVC(c) xPopMRC(context, Reg::RBP, c)
#define XPUSHMVC(c) xPushMRC(context, Reg::RBP, c)

#define XOUT() xOut(context)
#define XOUTC() xOutC(context)
#define XIN() xIn(context)

#define XCMPRR(r1, r2) xCmpRR(context, r1, r2)

//REGISTERS RBP RSP RDI RSI ARE BANNED
#define XCMPRC(r, c)   xCmpRC(context, r , c )

#define XEXIT() xExit(context)

const char* getRegName(Reg r);
const char* getReg8Name(Reg r);
const char* getFlagName(Flags f);

void xCqo(BackendContext* context);

void xPushR(BackendContext* context, Reg r);
void xPopR(BackendContext* context, Reg r);

void xPushMRC(BackendContext* context, Reg r, long long c);
void xPopMRC(BackendContext* context, Reg r, long long c);

void xCmpRR(BackendContext* context, Reg r1, Reg r2);
void xCmpRC(BackendContext* context, Reg r1, long long c);

void xExit(BackendContext* context);

void xIn(BackendContext* context);
void xOut(BackendContext* context);
void xOutC(BackendContext* context);

void xLeaRMRC(BackendContext* context, Reg r1, Reg r2, long long c);

void xRet(BackendContext* context);

void xSetR(BackendContext* context, Flags f, Reg r);

void xCallC(BackendContext* context, unsigned l);

void xJmpC(BackendContext* context, Flags f, unsigned l);

void xTestRR(BackendContext* context, Reg r1, Reg r2);

void xIncR(BackendContext* context, Reg r);
void xDecR(BackendContext* context, Reg r);

void xMovRR  (BackendContext* context, Reg r1, Reg r2);
void xMovRMCR(BackendContext* context, Reg r1, long long off, Reg r2);
void xMovRRMC(BackendContext* context, Reg r1, Reg r2, long long off);
void xMovRRM (BackendContext* context, Reg r1, Reg r2);
void xMovRMR (BackendContext* context, Reg r1, Reg r2);
void xMovRC  (BackendContext* context, Reg r, long long c);

void xBinary(BackendContext* context, unsigned char opcode, Reg r1, Reg r2);
void xIMulRR(BackendContext* context, Reg r1, Reg r2);
void xIDivR(BackendContext* context, Reg r);
void xShlRR(BackendContext* context, Reg r1, Reg r2);
void xShrRR(BackendContext* context, Reg r1, Reg r2);

#endif