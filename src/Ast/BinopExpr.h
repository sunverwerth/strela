// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstBinopExpr_h
#define Strela_Ast_AstBinopExpr_h

#include "Expr.h"

namespace Strela {

    class BinopExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::BinopExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TokenType op;
        Expr* left = nullptr;
        Expr* right = nullptr;
        FuncDecl* function = nullptr;
    };
}

#endif