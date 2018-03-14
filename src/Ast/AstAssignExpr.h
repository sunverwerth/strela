#ifndef Strela_Ast_AstAssignExpr_h
#define Strela_Ast_AstAssignExpr_h

#include "AstBinopExpr.h"

namespace Strela {
    class AstAssignExpr: public AstBinopExpr {
    public:
        AstAssignExpr(const Token& startToken, AstExpr* left, AstExpr* right): AstBinopExpr(startToken, left, right) {}
        STRELA_GET_TYPE(Strela::AstAssignExpr, Strela::AstBinopExpr);
        STRELA_IMPL_EXPR_VISITOR;
    };
}

#endif