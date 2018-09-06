// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstUnionTypeExpr_h
#define Strela_Ast_AstUnionTypeExpr_h

#include "Expr.h"

namespace Strela {

    class UnionTypeExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::UnionTypeExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* base = nullptr;
        Expr* next = nullptr;
    };
}

#endif