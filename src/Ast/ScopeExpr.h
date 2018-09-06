// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstScopeExpr_h
#define Strela_Ast_AstScopeExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class Symbol;

    class ScopeExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::ScopeExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* scopeTarget = nullptr;
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif