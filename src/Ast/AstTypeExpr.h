#ifndef Strela_Ast_AstTypeExpr_h
#define Strela_Ast_AstTypeExpr_h

#include "AstExpr.h"

namespace Strela {
    class AstTypeExpr: public AstExpr {
    public:
        AstTypeExpr(const Token& startToken): AstExpr(startToken) {}
        STRELA_GET_TYPE(Strela::AstTypeExpr, Strela::AstExpr);
    };
}

#endif