// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstMapLitExpr_h
#define Strela_Ast_AstMapLitExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class FuncDecl;
    class TypeDecl;
    
    class MapLitExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::MapLitExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::vector<Expr*> keys;
        std::vector<Expr*> values;
        FuncDecl* constructor = nullptr;
        TypeDecl* keyArrayType = &InvalidType::instance;
        TypeDecl* valueArrayType = &InvalidType::instance;
    };
}

#endif