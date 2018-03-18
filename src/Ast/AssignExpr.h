#ifndef Strela_Ast_AstAssignExpr_h
#define Strela_Ast_AstAssignExpr_h

#include "BinopExpr.h"

namespace Strela {
    class AssignExpr: public BinopExpr {
    public:
        AssignExpr(Expr* left, Expr* right): BinopExpr(TokenType::Equals, left, right) {}
        STRELA_GET_TYPE(Strela::AssignExpr, Strela::BinopExpr);
        STRELA_IMPL_EXPR_VISITOR;
    };
}

#endif