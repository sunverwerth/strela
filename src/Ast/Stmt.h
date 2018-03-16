#ifndef Strela_Ast_AstStmt_h
#define Strela_Ast_AstStmt_h

#include "Node.h"
#include "../IStmtVisitor.h"

#include <string>

#define STRELA_IMPL_STMT_VISITOR void accept(Strela::IStmtVisitor& v) override { v.visit(*this); }

namespace Strela {
    class Stmt: public Node {
    public:
        Stmt(const Token& startToken): Node(startToken) {}
        STRELA_GET_TYPE(Strela::Stmt, Strela::Node);
        virtual void accept(IStmtVisitor&) = 0;

    public:
        bool returns = false;
    };
}

#endif