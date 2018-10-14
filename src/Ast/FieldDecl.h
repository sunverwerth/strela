// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstFieldDecl_h
#define Strela_Ast_AstFieldDecl_h

#include "Stmt.h"
#include "InvalidType.h"

#include <string>

namespace Strela {
    class TypeDecl;
    class Expr;
    
    class FieldDecl: public Node {
    public:
        STRELA_GET_TYPE(Strela::FieldDecl, Strela::Node);

    public:
        std::string name;
        Expr* typeExpr = nullptr;
        TypeDecl* declType = &InvalidType::instance;

        int index = 0;
    };
}

#endif