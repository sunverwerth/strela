// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstGenericReificationExpr_h
#define Strela_Ast_AstGenericReificationExpr_h

#include "Expr.h"

#include <vector>

namespace Strela {

    class GenericReificationExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::GenericReificationExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* baseTypeExpr = nullptr;
        std::vector<Expr*> genericArguments;
    };
}

#endif