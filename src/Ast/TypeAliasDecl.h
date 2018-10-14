// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_TypeAliasDecl_h
#define Strela_Ast_TypeAliasDecl_h

#include "TypeDecl.h"

namespace Strela {
    class Expr;

    class TypeAliasDecl: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::TypeAliasDecl, Strela::TypeDecl);
        
		Node* getMember(const std::string& name) override;
		std::vector<Node*> getMethods(const std::string& name) override;

    public:
        Expr* typeExpr = nullptr;
        bool isExported = false;
    };
}

#endif