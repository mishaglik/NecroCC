#include "SyntaxAnalyzer.h"
#include "MGK/Logger.h"
#include "MGK/Utils.h"

#define REQUIRE(rule) ({                                                             \
    SyntaxContext _newContext = get ## rule(context);                                \
    if(_newContext.nParsed < 0) {                                                    \
        LOG_INFO("Unsuccessfull. Requirement \"%s\" not satisfied.", #rule);         \
        LOG_DEC_TAB(); return _newContext;                                           \
    }                                                                                \
    _newContext;                                                                     \
})

#define REQUIRE_OPR(opr) ({                                         \
    SyntaxContext _newContext = getOpr(context, opr);               \
    if(_newContext.nParsed < 0) {                                   \
        LOG_INFO("Requirement operator %s not satisfied.", #opr);   \
        RETURN(_newContext);                                        \
    }                                                               \
    _newContext;                                                    \
})

#define REQUIRE_CUSTOM(opr) ({                                      \
    SyntaxContext _newContext = getCustom(context, opr);            \
    if(_newContext.nParsed < 0) {                                   \
        LOG_INFO("Requirement operator %s not satisfied.", #opr);   \
        RETURN(_newContext);                                        \
    }                                                               \
    _newContext;                                                    \
})

#define INIT                                \
    context.nParsed = 0;                    \
    context.root = NULL;                    \
    LOG_ASSERT(context.curPtr != NULL);     \
    LOG_ASSERT(context.size > 0);           \
    LOG_INFO("Trying parse %s:", __func__); \
    LOG_INC_TAB();                           \
    LOG_INFO("Current node: %d:", context.curPtr - context.start)
    // nodeDump(*context.curPtr)                                     

#define RETURN(x)                                                           \
    LOG_INFO("%s parsing finished: ", __func__);                            \
    LOG_STYLE((x.nParsed < 0) ? ConsoleStyle::RED : ConsoleStyle::GREEN);   \
    LOG_INFO((x.nParsed < 0) ? "\b Failed" : "\bSuccessfull");              \
    LOG_DEC_TAB();                                                          \
    return x

#define TRY(rule)       {                               \
    LOG_INFO("Trying apply rule %s", #rule);            \
    SyntaxContext _newContext = get ## rule(context);   \
    if(_newContext.nParsed > 0){                        \
        LOG_INFO("Rule %s satisfied", #rule);           \
        RETURN(_newContext);                            \
    }                                                   \
}                                                       \

GRAMMAR_RULE(G){
    INIT;
    
    context = REQUIRE(Expr);

    if(context.size != 1 || (*context.curPtr)->type != NodeType::NONE){
        context.nParsed = -1;
        RETURN(context);
    }
    
    context.nParsed++;
    context.curPtr++;
    context.size--;
    RETURN(context);
}

GRAMMAR_RULE(Expr){
    INIT;

    Operator opList[1] = {Operator::ENDL};
    context = getOprLadder(context, opList, 1, &getLine);

    RETURN(context);
}

GRAMMAR_RULE(Line){
    INIT;
    
    TRY(DeclF);
    TRY(DeclV);
    TRY(Asg);
    TRY(Opt);

    LOG_INFO("");
    LOG_STYLE(ConsoleStyle::YELLOW);
    LOG_INFO("\bEmpty line found");
    RETURN(context);
}

GRAMMAR_RULE(DeclF){
    INIT;
    int nParsed = 0;

    context = REQUIRE_OPR(Operator::FUNC);
    nParsed += context.nParsed;

    Node* root = context.root;

    context = REQUIRE(Id);
    nParsed += context.nParsed;
    
    root->left = context.root;

    context = REQUIRE_CUSTOM(CustomOperator::PAR_L);
    nParsed += context.nParsed;

    Operator opList[1] =  {Operator::COMMA};
    context = getOprLadder(context, opList, 1, &getId);
    if(context.nParsed < 0){
        LOG_INFO("Arglist not satisfied\n");
        RETURN(context);
    }
    root->right = context.root;
    nParsed += context.nParsed;

    context = REQUIRE_CUSTOM(CustomOperator::PAR_R);
    nParsed += context.nParsed;

    context = REQUIRE_OPR(Operator::F_ARG);
    nParsed += context.nParsed;

    context.root->left = root->right;
    root->right = context.root;

    context = getLine(context);
    
    if(context.nParsed >= 0){
        root->right->right = context.root;
        nParsed += context.nParsed;
    }

    context.nParsed = nParsed;
    context.root    = root;
    RETURN(context);
}

GRAMMAR_RULE(DeclV){
    INIT;

    int nParsed = 0;
    Node* root  = NULL;

    context = REQUIRE_OPR(Operator::VAR);
    nParsed += context.nParsed;
    root = context.root;

    context = REQUIRE(Id);
    nParsed += context.nParsed;

    root->left = context.root;

    SyntaxContext newContext = getOpr(context, Operator::SET);

    if(newContext.nParsed < 0){
        context.root = root;
        context.nParsed = nParsed;
        RETURN(context);
    }

    newContext = getLine(newContext);
    if(newContext.nParsed < 0){
        context.root = root;
        context.nParsed = nParsed;
        RETURN(context);
    }
    
    root->right = newContext.root;
    
    context = newContext;

    context.nParsed += nParsed + 1;
    context.root    = root;
    RETURN(context);
}


GRAMMAR_RULE(Asg){
    INIT;

    Node* root = NULL;
    int nParsed = 0;

    context = REQUIRE(Id);
    nParsed += context.nParsed;
    root    = context.root;

    context = REQUIRE_OPR(Operator::SET);
    nParsed += context.nParsed;
    context.root->left = root;
    root = context.root;

    context = REQUIRE(Line);
    root->right = context.root;
    nParsed += context.nParsed;

    context.root = root;
    context.nParsed = nParsed;    

    RETURN(context);
}

GRAMMAR_RULE(Opt){
    INIT;

    context = REQUIRE(Rval);

    SyntaxContext newCont = getOpr(context, Operator::QQ);
    if(newCont.nParsed >= 0){
        Node* root = newCont.root;
        
        newCont = getLine(newCont);

        if(newCont.nParsed >= 0){
            root->left = context.root;
            root->right = newCont.root;
            newCont.root = root;
            newCont.nParsed += context.nParsed;
            RETURN(newCont);
        }

    }

    newCont = getOpr(context, Operator::TERN_Q);
    if(newCont.nParsed < 0)
        RETURN(context);
    
    Node* root = newCont.root;
    root->left = context.root;
    
    int nParse = context.nParsed + newCont.nParsed;

    context = getLine(newCont);
    if(context.nParsed < 0) RETURN(context);    
    nParse += context.nParsed;

    newCont = getOpr(context, Operator::TERN_C);
    if(newCont.nParsed < 0) RETURN(newCont);
    nParse += newCont.nParsed;

    root->right = newCont.root;
    root->right->left = context.root;

    context = getLine(newCont);
    if(context.nParsed < 0) RETURN(context);
    nParse += context.nParsed;

    root->right->left = context.root;

    context.nParsed = nParse;
    context.root = root;

    RETURN(context);
}

GRAMMAR_RULE(Rval){
    INIT;
    context = REQUIRE(Sum);
    RETURN(context);
}

GRAMMAR_RULE(Sum){
    INIT;

    Operator opList[2] = {Operator::ADD, Operator::SUB};
    context = getOprLadder(context, opList, 2, &getMul);

    if(context.nParsed <= 0){
        context.nParsed = -1;
    }
    RETURN(context);
}

GRAMMAR_RULE(Mul){
    INIT;

    Operator opList[2] = {Operator::MUL, Operator::DIV};
    context = getOprLadder(context, opList, 2, &getP);

    if(context.nParsed <= 0){
        context.nParsed = -1;
    }
    RETURN(context);
}

GRAMMAR_RULE(P){
    INIT;

    TRY(Wh);

    SyntaxContext newCont = getCustom(context, CustomOperator::PAR_L);
    if(newCont.nParsed >= 0){
        newCont = getExpr(newCont);
        if(newCont.nParsed < 0){
            RETURN(newCont);
        }
        Node* root = newCont.root;

        newCont = getCustom(newCont, CustomOperator::PAR_R);
        if(newCont.nParsed < 0){
            newCont.nParsed = -1;
            RETURN(newCont);
        }
        newCont.nParsed += 2;
        newCont.root = root;

        RETURN(newCont);
    }

    context = REQUIRE(Basic);
    RETURN(context);
}

GRAMMAR_RULE(Wh){
    INIT;
    Node* root = NULL;
    int nParse = 0;

    context = REQUIRE_OPR(Operator::WHILE);
    nParse += context.nParsed;
    root    = context.root;

    context = REQUIRE_CUSTOM(CustomOperator::PAR_L);
    nParse += context.nParsed;

    context = REQUIRE(Expr);
    nParse += context.nParsed;
    root->left = context.root;

    context = REQUIRE_CUSTOM(CustomOperator::PAR_R);
    nParse += context.nParsed;

    context = REQUIRE(Line);
    root->right = context.root;

    context.root = root;
    context.nParsed += nParse;
    RETURN(context);
}

GRAMMAR_RULE(Basic){
    INIT;
    TRY(Number);
    TRY(Call);
    TRY(Id);

    context.nParsed = -1;
    RETURN(context);
}

GRAMMAR_RULE(Call){
    INIT;
    int nParse = 0;
    Node* root = NULL;
    Node* left = NULL;

    context = REQUIRE(Id);
    left    = context.root;
    nParse += context.nParsed;
    
    context = REQUIRE_CUSTOM(CustomOperator::PAR_L);
    nParse += context.nParsed;
    root = context.root;

    root->left = left;

    Operator opList[1] = {Operator::COMMA};
    context = getOprLadder(context, opList, 1, &getLine);
    
    if(context.nParsed < 0) {
        RETURN(context);
        }

    nParse += context.nParsed;
    root->right =  context.root;

    context = REQUIRE_CUSTOM(CustomOperator::PAR_R);
    nParse += context.nParsed;

    LOG_WARNING("");
    LOG_STYLE(ConsoleStyle::YELLOW);
    LOG_WARNING("Call F. Changing node val");
    
    root->type = NodeType::OPERATOR;
    root->data.opr = Operator::CALL;

    context.nParsed = nParse;
    context.root  = root;

    RETURN(context);
}

SyntaxContext getOpr(SyntaxContext context, Operator opr){
    INIT;

    Node* node = *context.curPtr;
    if(node->type == NodeType::OPERATOR && node->data.opr == opr){
        node->left = NULL;
        node->right = NULL;

        context.root = *context.curPtr;
        context.curPtr++;
        context.size--;
        context.nParsed = 1;

        RETURN(context);
    }
    context.nParsed = -1;

    Node req = {.type = NodeType::OPERATOR, .data = {.opr = opr}};
    char* str = getNodeLabel(&req);
    LOG_INFO("Required operator: %s", str);
    free(str);


    RETURN(context);
}

SyntaxContext getCustom(SyntaxContext context, CustomOperator opr){
    INIT;

    Node* node = *context.curPtr;
    if(node->type == NodeType::CUSTOM && node->data.custom == opr){
        node->left = NULL;
        node->right = NULL;

        context.root = *context.curPtr;
        context.curPtr++;
        context.size--;
        context.nParsed = 1;
        RETURN(context);
    }

    Node req = {.type = NodeType::CUSTOM, .data = {.custom = opr}};
    char* str = getNodeLabel(&req);
    LOG_INFO("Required operator: %s", str);
    free(str);

    context.nParsed = -1;
    RETURN(context);
}

GRAMMAR_RULE(Number){
    INIT;

    Node* node = *context.curPtr;
    if(node->type == NodeType::NUMBER){
        node->left = NULL;
        node->right = NULL;

        context.root = *context.curPtr;
        context.curPtr++;
        context.size--;
        context.nParsed = 1;
        RETURN(context);
    }
    context.nParsed = -1;
    RETURN(context);
}

GRAMMAR_RULE(Id){
    INIT;

    Node* node = *context.curPtr;
    if(node->type == NodeType::IDENTIFIER){
        node->left = NULL;
        node->right = NULL;

        context.root = *context.curPtr;
        context.curPtr++;
        context.size--;
        context.nParsed = 1;
        // LOG_INFO("Found identifier with id = %d", node->data.id);
        RETURN(context);
    }
    context.nParsed = -1;
    RETURN(context);
}

SyntaxContext getOprLadder(SyntaxContext context, const Operator* oprRq, size_t n, SyntaxContext (*rule)(SyntaxContext context)){
    INIT;

    SyntaxContext newContext = (*rule)(context);
    if(context.nParsed < 0){
        LOG_INFO("Nothing found, but nothing required. So returning.");
        RETURN(context);
    }
    LOG_INFO("Found first element.");
    context = newContext;

    Node* lastOp = NULL;

    while(1){
        LOG_INFO("Trying to find {Opr Rule}* :");

        for(size_t i = 0; i < n; ++i){
            newContext = getOpr(context, oprRq[i]);
            if(newContext.nParsed >= 0) break;
        }
        
        if(newContext.nParsed < 0){
            LOG_INFO("Opr requirement not satisfied. Finishing ladder");
            break;
        }

        Node* opr = newContext.root;

        newContext = (*rule)(newContext);

        if(newContext.nParsed < 0){
            LOG_INFO("Rule requirement not satisfied. Finishing ladder");
            break;
        }

        LOG_INFO("Both requirements satisfied. Building tree.");
        if(lastOp == NULL){
            opr->left = context.root;
            context.root = opr;
            opr->right = newContext.root;
        }
        else{
            opr->left = lastOp->right;
            opr->right = newContext.root;
            lastOp->right = opr;
        }
            context.nParsed += newContext.nParsed + 1;
            context.curPtr  =  newContext.curPtr;
            context.size    =  newContext.size;
            lastOp = opr;
    }

    RETURN(context);
}