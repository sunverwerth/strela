// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstNewExpr_h
#define Strela_Ast_AstNewExpr_h

#include "Expr.h"

#include <string>
#include <vector>

namespace Strela {

    class NewExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::NewExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* typeExpr = nullptr;
        std::vector<Expr*> arguments;
        FuncDecl* initMethod = nullptr;
    };
}

#endif