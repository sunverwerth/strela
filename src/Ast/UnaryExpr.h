#ifndef Strela_Ast_AstUnaryExpr_h
#define Strela_Ast_AstUnaryExpr_h

#include "Expr.h"

namespace Strela {
    class UnaryExpr: public Expr {
    public:
        UnaryExpr(const Token& startToken, Expr* target): Expr(startToken), target(target) {}
        STRELA_GET_TYPE(Strela::UnaryExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* target;
    };
}

#endif