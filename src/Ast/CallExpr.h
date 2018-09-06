// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstCallExpr_h
#define Strela_Ast_AstCallExpr_h

#include "Expr.h"

#include <string>
#include <vector>

namespace Strela {
    class CallExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::CallExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* callTarget = nullptr;
        std::vector<Expr*> arguments;
    };
}

#endif