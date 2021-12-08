# Backend #

## Struct of code ##
bx -> beging of cur frame

table of vars: var -> offset

## Asm of Operarors ##

+ var: push[bx + offset]

+ decVar: Create offset + (Eval(R) pop[bx + offset])

+ +-*/%... EVAL(L) EVAL(R) add/sub/mul/

+ SET EVAL(R) pop[bx + offset]

+ ENDL if(L) EVAL(L); if(R) {if(L) pop; EVAL(R)};

+ QQ EVAL(L) pop dx push dx push dx push 0 if(dx)

+ Q EVAL(L) if (L) EVAL(R->L); else EVAL(R->R);

+ WHILE 

+ CALL

