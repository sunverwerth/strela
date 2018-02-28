#ifndef Strela_Ast_AstIdExpr_h
#define Strela_Ast_AstIdExpr_h

#include "AstExpr.h"

#include <string>

namespace Strela {
    class Symbol;

    class AstIdExpr: public AstExpr {
    public:
        AstIdExpr(const Token& startToken, const std::string& name): AstExpr(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::AstIdExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif