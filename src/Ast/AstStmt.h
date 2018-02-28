#ifndef Strela_Ast_AstStmt_h
#define Strela_Ast_AstStmt_h

#include "AstNode.h"

#include <string>

namespace Strela {
    class AstStmt: public AstNode {
    public:
        AstStmt(const Token& startToken): AstNode(startToken) {}
        STRELA_GET_TYPE(Strela::AstStmt, Strela::AstNode);

    public:
        bool returns = false;
    };
}

#endif