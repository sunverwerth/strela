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
        CastExpr(Expr* sourceExpr, TypeDecl* targetType): Expr(), sourceExpr(sourceExpr), targetType(targetType) {}
        CastExpr(Expr* sourceExpr, TypeExpr* targetTypeExpr): Expr(), sourceExpr(sourceExpr), targetTypeExpr(targetTypeExpr) {}
        STRELA_GET_TYPE(Strela::CastExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* sourceExpr;
        TypeExpr* targetTypeExpr = nullptr;
        TypeDecl* targetType = nullptr;
        Implementation* implementation = nullptr;
    };
}

#endif