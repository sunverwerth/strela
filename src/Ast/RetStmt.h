#ifndef Strela_Ast_AstRetStmt_h
#define Strela_Ast_AstRetStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class RetStmt: public Stmt {
    public:
        RetStmt(const Token& startToken, Expr* expression): Stmt(startToken), expression(expression) {}
        STRELA_GET_TYPE(Strela::RetStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* expression;
    };
}

#endif