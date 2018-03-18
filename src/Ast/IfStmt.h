#ifndef Strela_Ast_AstIfStmt_h
#define Strela_Ast_AstIfStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;
    class IStmtVisitor;

    class IfStmt: public Stmt {
    public:
        IfStmt(
            Expr* condition,
            Stmt* trueBranch,
            Stmt* falseBranch
        ): Stmt(), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch) {}
        STRELA_GET_TYPE(Strela::IfStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* condition;
        Stmt* trueBranch;
        Stmt* falseBranch;
    };
}

#endif