#ifndef Strela_Ast_AstArrayTypeExpr_h
#define Strela_Ast_AstArrayTypeExpr_h

#include "AstTypeExpr.h"

namespace Strela {

    class AstArrayTypeExpr: public AstTypeExpr {
    public:
        AstArrayTypeExpr(const Token& startToken, AstTypeExpr* base): AstTypeExpr(startToken), base(base) {}
        STRELA_GET_TYPE(Strela::AstArrayTypeExpr, Strela::AstTypeExpr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        AstTypeExpr* base;
    };
}

#endif