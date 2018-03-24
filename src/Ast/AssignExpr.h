// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstAssignExpr_h
#define Strela_Ast_AstAssignExpr_h

#include "BinopExpr.h"

namespace Strela {
    class AssignExpr: public BinopExpr {
    public:
        AssignExpr(TokenType op, Expr* left, Expr* right): BinopExpr(op, left, right) {}
        STRELA_GET_TYPE(Strela::AssignExpr, Strela::BinopExpr);
        STRELA_IMPL_EXPR_VISITOR;
    };
}

#endif