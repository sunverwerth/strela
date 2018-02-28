#ifndef Strela_Ast_AstUnaryExpr_h
#define Strela_Ast_AstUnaryExpr_h

#include "AstExpr.h"

namespace Strela {
    class AstUnaryExpr: public AstExpr {
    public:
        AstUnaryExpr(const Token& startToken, AstExpr* target): AstExpr(startToken), target(target) {}
        STRELA_GET_TYPE(Strela::AstUnaryExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* target;
    };
}

#endif