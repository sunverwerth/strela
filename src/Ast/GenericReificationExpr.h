#ifndef Strela_Ast_AstGenericReificationExpr_h
#define Strela_Ast_AstGenericReificationExpr_h

#include "TypeExpr.h"

#include <vector>

namespace Strela {

    class GenericReificationExpr: public TypeExpr {
    public:
        GenericReificationExpr(TypeExpr* base, const std::vector<TypeExpr*>& genericArguments): TypeExpr(), base(base), genericArguments(genericArguments) {}
        STRELA_GET_TYPE(Strela::GenericReificationExpr, Strela::TypeExpr);
        STRELA_IMPL_TYPE_EXPR_VISITOR;

    public:
        TypeExpr* base;
        std::vector<TypeExpr*> genericArguments;
    };
}

#endif