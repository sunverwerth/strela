// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstIfStmt_h
#define Strela_Ast_AstIfStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class IfStmt: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::IfStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* condition = nullptr;
        Stmt* trueBranch = nullptr;
        Stmt* falseBranch = nullptr;
    };
}

#endif