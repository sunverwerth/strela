#ifndef Strela_Ast_AstIdTypeExpr_h
#define Strela_Ast_AstIdTypeExpr_h

#include "AstTypeExpr.h"

#include <string>

namespace Strela {
    class Symbol;

    class AstIdTypeExpr: public AstTypeExpr {
    public:
        AstIdTypeExpr(const Token& startToken, const std::string& name): AstTypeExpr(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::AstIdTypeExpr, Strela::AstTypeExpr);
        STRELA_IMPL_VISITOR;

    public:
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif