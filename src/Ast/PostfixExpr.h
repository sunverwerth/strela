// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstPostfixExpr_h
#define Strela_Ast_AstPostfixExpr_h

#include "Expr.h"

namespace Strela {
    class PostfixExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::PostfixExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TokenType op;
        Expr* target = nullptr;
    };
}

#endif