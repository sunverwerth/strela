// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstCastExpr_h
#define Strela_Ast_AstCastExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class Implementation;
    
    class CastExpr: public Expr {
    public:
        STRELA_GET_TYPE(Strela::CastExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* sourceExpr = nullptr;
        Expr* targetTypeExpr = nullptr;
        TypeDecl* targetType = nullptr;
        Implementation* implementation = nullptr;
    };
}

#endif