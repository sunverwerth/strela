// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstExprStmt_h
#define Strela_Ast_AstExprStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class ExprStmt: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::ExprStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* expression = nullptr;
    };
}

#endif