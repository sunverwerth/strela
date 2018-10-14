// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstParam_h
#define Strela_Ast_AstParam_h

#include "Node.h"
#include "InvalidType.h"

#include <string>

namespace Strela {
    class Expr;
    class TypeDecl;
    
    class Param: public Node {
    public:
        STRELA_GET_TYPE(Strela::Param, Strela::Node);

    public:
        int index = 0;
        std::string name;
        Expr* typeExpr = nullptr;
        TypeDecl* declType = &InvalidType::instance;
    };
}

#endif