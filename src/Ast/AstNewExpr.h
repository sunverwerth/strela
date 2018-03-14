#ifndef Strela_Ast_AstNewExpr_h
#define Strela_Ast_AstNewExpr_h

#include "AstExpr.h"

#include <string>

namespace Strela {
    class Type;
    class AstTypeExpr;

    class AstNewExpr: public AstExpr {
    public:
        AstNewExpr(const Token& startToken, AstTypeExpr* typeExpr): AstExpr(startToken), typeExpr(typeExpr) {}
        STRELA_GET_TYPE(Strela::AstNewExpr, Strela::AstExpr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        AstTypeExpr* typeExpr;
    };
}

#endif