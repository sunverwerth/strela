#ifndef Strela_Ast_AstUnionTypeExpr_h
#define Strela_Ast_AstUnionTypeExpr_h

#include "TypeExpr.h"

namespace Strela {

    class UnionTypeExpr: public TypeExpr {
    public:
        UnionTypeExpr(TypeExpr* base, TypeExpr* next): TypeExpr(), base(base), next(next) {}
        STRELA_GET_TYPE(Strela::UnionTypeExpr, Strela::TypeExpr);
        STRELA_IMPL_TYPE_EXPR_VISITOR;

    public:
        TypeExpr* base;
        TypeExpr* next;
    };
}

#endif