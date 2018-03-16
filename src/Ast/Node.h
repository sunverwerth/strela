#ifndef Strela_Ast_AstNode_h
#define Strela_Ast_AstNode_h

#include "../TypeInfo.h"
#include "Token.h"

#include <iostream>

namespace Strela {
    class Node {
    public:
        Node(const Token& startToken): startToken(startToken) {}
        virtual ~Node() {}
        STRELA_BASE_TYPE(Strela::Node);

    public:
        Token startToken;
    };
}
#endif