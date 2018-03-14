#ifndef Strela_Ast_AstStmt_h
#define Strela_Ast_AstStmt_h

#include "AstNode.h"
#include "../IStmtVisitor.h"

#include <string>

#define STRELA_IMPL_STMT_VISITOR void accept(Strela::IStmtVisitor& v) override { v.visit(*this); }

namespace Strela {
    class AstStmt: public AstNode {
    public:
        AstStmt(const Token& startToken): AstNode(startToken) {}
        STRELA_GET_TYPE(Strela::AstStmt, Strela::AstNode);
        virtual void accept(IStmtVisitor&) = 0;

    public:
        bool returns = false;
    };
}

#endif