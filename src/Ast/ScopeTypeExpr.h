// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstScopeTypeExpr_h
#define Strela_Ast_AstScopeTypeExpr_h

#include "TypeExpr.h"

#include <string>

namespace Strela {
    class Symbol;

    class ScopeTypeExpr: public TypeExpr {
    public:
        ScopeTypeExpr(TypeExpr* target, const std::string& name): TypeExpr(), target(target), name(name) {}
        STRELA_GET_TYPE(Strela::ScopeTypeExpr, Strela::TypeExpr);
        STRELA_IMPL_TYPE_EXPR_VISITOR;

    public:
        TypeExpr* target;
        std::string name;
    };
}

#endif