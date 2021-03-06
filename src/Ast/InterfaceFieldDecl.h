// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstInterfaceFieldDecl_h
#define Strela_Ast_AstInterfaceFieldDecl_h

#include "Node.h"
#include "InvalidType.h"

#include <string>

namespace Strela {
    class Expr;
    class TypeDecl;

    class InterfaceFieldDecl: public Node {
    public:
        STRELA_GET_TYPE(Strela::InterfaceFieldDecl, Strela::Node);
        std::string getDescription();

    public:
        std::string name;
        Expr* typeExpr = nullptr;
        TypeDecl* declType = &InvalidType::instance;

        int index = 0;
    };
}

#endif