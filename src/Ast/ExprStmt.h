#ifndef Strela_Ast_AstExprStmt_h
#define Strela_Ast_AstExprStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class ExprStmt: public Stmt {
    public:
        ExprStmt(Expr* expression): Stmt(), expression(expression) {}
        STRELA_GET_TYPE(Strela::ExprStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* expression;
    };
}

#endif