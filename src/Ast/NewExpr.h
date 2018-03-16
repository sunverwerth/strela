#ifndef Strela_Ast_AstNewExpr_h
#define Strela_Ast_AstNewExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class Type;
    class TypeExpr;

    class NewExpr: public Expr {
    public:
        NewExpr(const Token& startToken, TypeExpr* typeExpr): Expr(startToken), typeExpr(typeExpr) {}
        STRELA_GET_TYPE(Strela::NewExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TypeExpr* typeExpr;
    };
}

#endif