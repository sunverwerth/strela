#ifndef Strela_Ast_AstCallExpr_h
#define Strela_Ast_AstCallExpr_h

#include "Expr.h"

#include <string>
#include <vector>

namespace Strela {
    class CallExpr: public Expr {
    public:
        CallExpr(const Token& startToken, Expr* callTarget, const std::vector<Expr*>& arguments): Expr(startToken), callTarget(callTarget), arguments(arguments) {}
        STRELA_GET_TYPE(Strela::CallExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* callTarget;
        std::vector<Expr*> arguments;
    };
}

#endif