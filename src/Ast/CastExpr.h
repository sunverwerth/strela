#ifndef Strela_Ast_AstCastExpr_h
#define Strela_Ast_AstCastExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class Implementation;
    
    class CastExpr: public Expr {
    public:
        CastExpr(Expr* sourceExpr, TypeDecl* targetType): Expr(), sourceExpr(sourceExpr), targetType(targetType) {}
        STRELA_GET_TYPE(Strela::CastExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* sourceExpr;
        TypeDecl* targetType;
        Implementation* implementation = nullptr;
    };
}

#endif