#ifndef Strela_Ast_AstBinopExpr_h
#define Strela_Ast_AstBinopExpr_h

#include "AstExpr.h"

namespace Strela {

    class AstBinopExpr: public AstExpr {
    public:
        AstBinopExpr(const Token& startToken, AstExpr* left, AstExpr* right): AstExpr(startToken), left(left), right(right) {}
        STRELA_GET_TYPE(Strela::AstBinopExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* left;
        AstExpr* right;
    };
}

#endif