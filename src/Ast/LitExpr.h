// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstLitExpr_h
#define Strela_Ast_AstLitExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class LitExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::LitExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Token token;
    };
}

#endif