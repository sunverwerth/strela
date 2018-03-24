// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstNullableTypeExpr_h
#define Strela_Ast_AstNullableTypeExpr_h

#include "TypeExpr.h"

namespace Strela {
    class NullableTypeExpr: public TypeExpr {
    public:
        NullableTypeExpr(TypeExpr* base): TypeExpr(), base(base) {}
        STRELA_GET_TYPE(Strela::NullableTypeExpr, Strela::TypeExpr);
        STRELA_IMPL_TYPE_EXPR_VISITOR;

    public:
        TypeExpr* base;
    };
}

#endif