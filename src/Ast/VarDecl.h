// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstVarDecl_h
#define Strela_Ast_AstVarDecl_h

#include "Stmt.h"
#include "InvalidType.h"

#include <string>

namespace Strela {
    class Expr;
    class TypeDecl;
    
    class VarDecl: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::VarDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        Expr* typeExpr = nullptr;
        Expr* initializer = nullptr;
        TypeDecl* declType = &InvalidType::instance;

        int index = 0;
    };
}

#endif