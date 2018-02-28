#ifndef Strela_Ast_AstPostfixExpr_h
#define Strela_Ast_AstPostfixExpr_h

#include "AstExpr.h"

namespace Strela {
    class AstPostfixExpr: public AstExpr {
    public:
        AstPostfixExpr(const Token& startToken, AstExpr* target): AstExpr(startToken), target(target) {}
        STRELA_GET_TYPE(Strela::AstPostfixExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* target;
    };
}

#endif