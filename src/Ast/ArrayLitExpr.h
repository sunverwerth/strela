#ifndef Strela_Ast_AstArrayLitExpr_h
#define Strela_Ast_AstArrayLitExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class ArrayLitExpr: public Expr {
    public:
        ArrayLitExpr(const std::vector<Expr*>& elements): Expr(), elements(elements) {}
        STRELA_GET_TYPE(Strela::ArrayLitExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::vector<Expr*> elements;
    };
}

#endif