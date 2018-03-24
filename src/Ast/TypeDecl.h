// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_TypeDecl_h
#define Strela_Ast_TypeDecl_h

#include "Stmt.h"

#include <string>
#include <vector>

namespace Strela {
    class FuncDecl;
    class TypeDecl: public Stmt {
    public:
        TypeDecl(const std::string& name): Stmt(), name(name) {}
        STRELA_GET_TYPE(Strela::TypeDecl, Strela::Stmt);
        virtual Node* getMember(const std::string& name) { return nullptr; }
        virtual std::vector<Node*> getMethods(const std::string& name) { return {}; }

    public:
        std::string name;
    };
}

#endif