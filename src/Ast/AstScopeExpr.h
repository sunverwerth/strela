#ifndef Strela_Ast_AstScopeExpr_h
#define Strela_Ast_AstScopeExpr_h

#include "AstExpr.h"

#include <string>

namespace Strela {
    class Symbol;

    class AstScopeExpr: public AstExpr {
    public:
        AstScopeExpr(const Token& startToken, AstExpr* scopeTarget, const std::string& name): AstExpr(startToken), scopeTarget(scopeTarget), name(name) {}
        STRELA_GET_TYPE(Strela::AstScopeExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* scopeTarget;
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif