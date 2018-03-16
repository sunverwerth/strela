#ifndef Strela_Ast_AstTypeExpr_h
#define Strela_Ast_AstTypeExpr_h

#include "Expr.h"

namespace Strela {
    class TypeExpr: public Expr {
    public:
        TypeExpr(const Token& startToken): Expr(startToken) {}
        STRELA_GET_TYPE(Strela::TypeExpr, Strela::Expr);
    };
}

#endif