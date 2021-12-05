#ifndef STD_LANGTREE_H
#define STD_LANGTREE_H

#define STD_LANG_VERSION_MAJOR 1
#define STD_LAND_VERSION_MINOR 0

enum class NodeType{
    NONE       = 0,
    NUMBER     = 1,
    IDENTIFIER = 2,
    OPERATOR   = 3,
    CUSTOM     = 4,     // Only for temporary nodes. Not standart node type. 
};

#ifndef CUSTOM_NODE_DATA_TYPE
#define CUSTOM_NODE_DATA_TYPE int
#endif

/**
 * @warning Numeration of Operator can be changed on next versions. So do NOT relate on values.
 * @brief
 */

enum class Operator{
// ##### Incorrect oprerator #####
    NONE    = 0,

// ##### Arithm operators #####
    ADD     = 1,    // L + R                                                            [basic]
    SUB     = 2,    // L - R                                                            [basic]
    MUL     = 3,    // L * R                                                            [basic]
    DIV     = 4,    // L / R                                                            [basic]
    MOD     = 5,    // L % R                                                            [arithm]
    AND     = 6,    // L & R                                                            [arithm]
    OR      = 7,    // L | R                                                            [arithm]
    XOR     = 8,    // L ^ R                                                            [arithm]
    SHL     = 9,    // L << R                                                           [arithm]
    SHR     = 10,   // L >> R                                                           [arithm]

// ##### ASSIGNMENT #####
    SET     = 11,   // L := R (Set L value to R).                                       [basic]

// ##### COMPARE #####  
//!
//! @brief Return 0 or 1.
//!
    EQUAL   = 12,   // L == R                                                           [basic]
    NON_EQ  = 13,   // L != R                                                           [basic]
    LESS    = 14,   // L < R                                                            [basic]
    GRTR    = 15,   // L > R                                                            [basic]
    LESS_EQ = 16,   // L <= R                                                           [basic]
    GRTR_EQ = 17,   // L >= R                                                           [basic]

//  ##### UNARY #####
    NOT     = 18,  // !R                                                                [basic]
    INC     = 19,  // L++ or ++R                                                        [unary]
    DEC     = 20,  // L-- or --R                                                        [unary]

// ##### VAR & FUNC #####
    VAR     = 21,  // Declares L as local variable with R as value                        [basic]
    FUNC    = 22,  // Declares L as function with R-> as param tree (comma separated).    [basic]
    F_ARG   = 23,  // Declare function sepaerator. L is param tree. R is code             [basic]
    COMMA   = 24,  // Function argument separator.                                      [multicall]
    CALL    = 25,  // Call L with R as paraments (comma separated).                     [basic]
    RET     = 26,  // Return from fuction. 

// ##### SEPARATORS #####
    ENDL    = 27, // L; R - if R returns R, else if L retunrs L else 0.                 [basic]
    QQ      = 28, // L ?? R - if L != 0 return L, else R                                [qq]
    TERN_Q  = 29, // L ? (R->L) : (R->R)  - if L return R->L else R->R                  [basic]
    TERN_C  = 30, // -----------^                                                       [basic]

// ##### LOOP #####
    WHILE   = 31, // while(L) R                                                         [basic]
// ? BREAK = 32,

// ##### MEMORY #####
    ADDR    = 33, // return adress of R(VAR).                                           [memory]
    VAL     = 34, // return value  of R. (From memory).                                 [memory]

// ##### DIFFERENTIATE #####
    DIFF    = 35, // (R)'. R must differable.                                           [basic]
};


typedef int      num_t;
typedef Operator opr_t;
typedef int      idt_t;

union NodeData
{
    num_t num;
    opr_t opr;
    idt_t  id;

    CUSTOM_NODE_DATA_TYPE custom;
};


struct Node
{
    NodeType type = NodeType::NONE;
    NodeData data = {0};

    Node* left  = 0;
    Node* right = 0;

    #ifdef NODE_DEBUG_INFO
    char* name = 0;
    int   line = 0;
    #endif 
};


#endif