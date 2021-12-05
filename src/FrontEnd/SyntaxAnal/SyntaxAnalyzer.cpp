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
    _newContext;
})

#define INIT                                \
    context.nParsed = 0;                    \
    context.root = NULL;                    \
    LOG_ASSERT(context.curPtr != NULL);     \
    LOG_ASSERT(context.size > 0);           \
    LOG_INFO("Trying parse %s:", __func__); \
    LOG_INC_TAB()                           \

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

    if(context.size != 1){
        context.nParsed = -1;
    }

    RETURN(context);
}

GRAMMAR_RULE(Expr){
    INIT;

    context = getOprLadder(context, Operator::ENDL, &getLine);

    RETURN(context);
}

GRAMMAR_RULE(Line){
    INIT;

    SyntaxContext newContext = {};
    
    TRY(DeclF);
    TRY(DeclV);
    TRY(Asg);
    TRY(Opt);

    LOG_INFO("");
    LOG_STYLE(ConsoleStyle::YELLOW);
    LOG_INFO("\bEmpty line found");
    RETURN(context);
}

GRAMMAR_RULE(Asg){
    INIT;

    REQUIRE(Id);

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

        RETURN(context)
    }
    context.nParsed = -1;
    return(context);
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

SyntaxContext getOprLadder(SyntaxContext context, Operator oprRq, SyntaxContext (*rule)(SyntaxContext context)){
    INIT;

    SyntaxContext newContext = (*rule)(context);
    if(context.nParsed < 0){
        LOG_INFO("Nothing found, but nothing required. So returning.");
        RETURN(context);
    }
    LOG_INFO("Found first element.");

    Node* lastOp = NULL;

    while(1){
        LOG_INFO("Trying to find {Opr Rule}* :");

        newContext = getOpr(context, oprRq);
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