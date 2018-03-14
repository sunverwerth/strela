#ifndef Strela_Ast_AstNode_h
#define Strela_Ast_AstNode_h

#include "../TypeInfo.h"
#include "Token.h"

#include <iostream>

namespace Strela {
    class AstNode {
    public:
        AstNode(const Token& startToken): startToken(startToken) {}
        virtual ~AstNode() {}
        STRELA_BASE_TYPE(Strela::AstNode);

    public:
        Token startToken;
    };
}
#endif