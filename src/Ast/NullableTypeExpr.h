// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstNullableTypeExpr_h
#define Strela_Ast_AstNullableTypeExpr_h

#include "Expr.h"

namespace Strela {
    class NullableTypeExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::NullableTypeExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* baseTypeExpr = nullptr;
    };
}

#endif