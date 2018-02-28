#ifndef Strela_Ast_AstCallExpr_h
#define Strela_Ast_AstCallExpr_h

#include "AstExpr.h"

#include <string>
#include <vector>

namespace Strela {
    class AstCallExpr: public AstExpr {
    public:
        AstCallExpr(const Token& startToken, AstExpr* callTarget, const std::vector<AstExpr*>& arguments): AstExpr(startToken), callTarget(callTarget), arguments(arguments) {}
        STRELA_GET_TYPE(Strela::AstCallExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* callTarget;
        std::vector<AstExpr*> arguments;
    };
}

#endif