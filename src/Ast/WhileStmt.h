// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstWhileStmt_h
#define Strela_Ast_AstWhileStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class WhileStmt: public Stmt {
    public:
        WhileStmt(Expr* condition, Stmt* body): Stmt(), condition(condition), body(body) {}
        STRELA_GET_TYPE(Strela::WhileStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* condition;
        Stmt* body;
    };
}

#endif