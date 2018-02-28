#ifndef Strela_Ast_AstNode_h
#define Strela_Ast_AstNode_h

#include "../TypeInfo.h"
#include "../NodeVisitor.h"
#include "Token.h"

#include <iostream>

#define STRELA_IMPL_VISITOR void accept(Strela::NodeVisitor& v) override { v.visit(*this); }

namespace Strela {
    class AstNode {
    public:
        AstNode(const Token& startToken): startToken(startToken) {}
        virtual ~AstNode() {}
        STRELA_BASE_TYPE(Strela::AstNode);
        virtual void accept(NodeVisitor& v) = 0;

    public:
        Token startToken;
    };
}
#endif