// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstBlockStmt_h
#define Strela_Ast_AstBlockStmt_h

#include "Stmt.h"

#include <vector>

namespace Strela {
    class BlockStmt: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::BlockStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::vector<Stmt*> stmts;
    };
}
#endif