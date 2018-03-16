#ifndef Strela_Ast_AstPostfixExpr_h
#define Strela_Ast_AstPostfixExpr_h

#include "Expr.h"

namespace Strela {
    class PostfixExpr: public Expr {
    public:
        PostfixExpr(const Token& startToken, Expr* target): Expr(startToken), target(target) {}
        STRELA_GET_TYPE(Strela::PostfixExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* target;
    };
}

#endif