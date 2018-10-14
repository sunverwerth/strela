// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstArrayLitExpr_h
#define Strela_Ast_AstArrayLitExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class FuncDecl;
    
    class ArrayLitExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::ArrayLitExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::vector<Expr*> elements;
        FuncDecl* constructor = nullptr;
    };
}

#endif