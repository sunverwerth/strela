#ifndef Strela_Ast_AstArrayTypeExpr_h
#define Strela_Ast_AstArrayTypeExpr_h

#include "TypeExpr.h"

namespace Strela {

    class ArrayTypeExpr: public TypeExpr {
    public:
        ArrayTypeExpr(const Token& startToken, TypeExpr* base): TypeExpr(startToken), base(base) {}
        STRELA_GET_TYPE(Strela::ArrayTypeExpr, Strela::TypeExpr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TypeExpr* base;
    };
}

#endif