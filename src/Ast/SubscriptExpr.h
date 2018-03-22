#ifndef Strela_Ast_AstSubscriptExpr_h
#define Strela_Ast_AstSubscriptExpr_h

#include "Expr.h"

#include <string>
#include <vector>

namespace Strela {
    class SubscriptExpr: public Expr {
    public:
        SubscriptExpr(Expr* callTarget, const std::vector<Expr*>& arguments): Expr(), callTarget(callTarget), arguments(arguments) {}
        STRELA_GET_TYPE(Strela::SubscriptExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* callTarget;
        std::vector<Expr*> arguments;
        FuncDecl* subscriptFunction = nullptr;
    };
}

#endif