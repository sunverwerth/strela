#ifndef Strela_Ast_AstPostfixExpr_h
#define Strela_Ast_AstPostfixExpr_h

#include "Expr.h"

namespace Strela {
    class PostfixExpr: public Expr {
    public:
        PostfixExpr(Expr* target, TokenType op): Expr(), target(target), op(op) {}
        STRELA_GET_TYPE(Strela::PostfixExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TokenType op;
        Expr* target;
    };
}

#endif