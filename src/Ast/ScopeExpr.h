#ifndef Strela_Ast_AstScopeExpr_h
#define Strela_Ast_AstScopeExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class Symbol;

    class ScopeExpr: public Expr {
    public:
        ScopeExpr(Expr* scopeTarget, const std::string& name): Expr(), scopeTarget(scopeTarget), name(name) {}
        STRELA_GET_TYPE(Strela::ScopeExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Expr* scopeTarget;
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif