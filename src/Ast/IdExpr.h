// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstIdExpr_h
#define Strela_Ast_AstIdExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class IdExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::IdExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::string name;
    };
}

#endif