// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstRetStmt_h
#define Strela_Ast_AstRetStmt_h

#include "Stmt.h"

namespace Strela {
    class Expr;

    class RetStmt: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::RetStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        Expr* expression = nullptr;
    };
}

#endif