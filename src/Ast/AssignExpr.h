#ifndef Strela_Ast_AstAssignExpr_h
#define Strela_Ast_AstAssignExpr_h

#include "BinopExpr.h"

namespace Strela {
    class AssignExpr: public BinopExpr {
    public:
        AssignExpr(const Token& startToken, Expr* left, Expr* right): BinopExpr(startToken, left, right) {}
        STRELA_GET_TYPE(Strela::AssignExpr, Strela::BinopExpr);
        STRELA_IMPL_EXPR_VISITOR;
    };
}

#endif