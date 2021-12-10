# Parser generator #
## Syntax ##
1. ### Parser ###
    Basic rules analyzer:
    Number{
        [0-9]
    }


2. ### Syntax analyzer ### 
    Rule ::= Amount*
    Amount ::= Low [*+?]
    Low    ::= {Rule} | Complex
    Compex ::= 
    Basic

    %R = %1
    %R->left = %2
    
    Summ ::= Mul {[+-] Mul}*
    %2 := %2.1
    %2->Root->left := %2.2
    %2* := right
    % := %2 ?? %1
    %->left := %2 ? %1 : NULL
    %->right := %2 ?? NULL