# Grammatic rules #

## Basic rules ##
- **N** - number
- **V** - variable
- **F** - function




## Operator rules ##
- 
- G ::= Expr$      //Grammar
- 
- Expr ::= Line {; Line}*      // Expression
- 
- Line ::= DeclF | DeclV | Asg | Opt
- 
- Asg ::= Id = Line
- 
- Opt  ::= Rval {?? Line | ? Line : Line }?
- 
- Rval ::= Sum 
-  
- Summ ::= Mul {+- Mul} *
- 
- Mul  ::= P   {*/ P  } *
- 
- P ::= Wh | (Expr) | Basic
- 
- Wh ::= while ( E ) Line
- 
- Basic ::= Number | Call | Id
- 
- Call ::= Id (Rval? {, Rval}*)
- 
- DeclF ::= DecF Id ({Id {, Id}*}?)
- 
- DeclV ::= DecV Id (= Line)?



// ##### Arithm operators #####
    MUL     = 3,    // L * R                                                            [basic]
    DIV     = 4,    // L / R                                                            [basic]
    MOD     = 5,    // L % R                                                            [arithm]
    AND     = 6,    // L & R                                                            [arithm]
    OR      = 7,    // L | R                                                            [arithm]
    XOR     = 8,    // L ^ R                                                            [arithm]
    SHL     = 9,    // L << R                                                           [arithm]
    SHR     = 10,   // L >> R                                                           [arithm]


// ##### COMPARE #####  
    EQUAL   = 12,   // L == R                                                           [compare]
    NON_EQ  = 13,   // L != R                                                           [compare]
    LESS    = 14,   // L < R                                                            [compare]
    GRTR    = 15,   // L > R                                                            [compare]
    LESS_EQ = 16,   // L <= R                                                           [compare]
    GRTR_EQ = 17,   // L >= R                                                           [compare]

//  ##### UNARY #####
    NOT     = 18,  // !R                                                                [compare]
    INC     = 19,  // L++ or ++R                                                        [unary]
    DEC     = 20,  // L-- or --R                                                        [unary]

// ##### VAR & FUNC #####
    VAR     = 21,  // Declares L as local variable with R as value                      [basic]
    FUNC    = 22,  // Declares L as function with R as param tree (comma separated).    [basic]
    COMMA   = 23,  // Function argument separator.                                      [multicall]
// ? RET    = 25,  // Return from fuction. 

// ##### SEPARATORS #####
    QQ      = 27, // L ?? R - if L != 0 return L, else R                                [qq]
    TERN_Q  = 28, // L ? (R->L) : (R->R)  - if L return R->L else R->R                  [basic]
    TERN_C  = 29, // -----------^                                                       [basic]

// ##### LOOP #####
    WHILE   = 30, // while(L) R                                                         [basic]
// ? BREAK = 31,

// ##### MEMORY #####
    ADDR    = 32, // return adress of R(VAR).                                           [memory]
    VAL     = 33, // return value  of R. (From memory).                                 [memory]

// ##### DIFFERENTIATE #####
    DIFF    = 34, // (R)'. R must differable.                                           [basic]

//########## Released #############
    ENDL    = 26, // L; R - if R returns R, else if L retunrs L else 0.                 [basic]
    CALL    = 24,  // Call L with R as paraments (comma separated).                     [basic]
/
    ADD     = 1,    // L + R                                                            [basic]
    SUB     = 2,    // L - R                                                            [basic]

// ##### ASSIGNMENT #####
    SET     = 11,   // L := R (Set L value to R).                                       [basic]