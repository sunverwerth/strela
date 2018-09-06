// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstStmt_h
#define Strela_Ast_AstStmt_h

#include "Node.h"
#include "../IStmtVisitor.h"

#include <string>

#define STRELA_IMPL_STMT_VISITOR void accept(Strela::IStmtVisitor<void>& v) override { return v.visit(*this); }

namespace Strela {
    class Stmt: public Node {
    public:
        STRELA_GET_TYPE(Strela::Stmt, Strela::Node);
        virtual void accept(IStmtVisitor<void>&) = 0;

    public:
        bool returns = false;
    };
}

#endif