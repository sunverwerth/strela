#ifndef Strela_Ast_AstNewExpr_h
#define Strela_Ast_AstNewExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class TypeExpr;

    class NewExpr: public Expr {
    public:
        NewExpr(TypeExpr* typeExpr): Expr(), typeExpr(typeExpr) {}
        STRELA_GET_TYPE(Strela::NewExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TypeExpr* typeExpr;
    };
}

#endif