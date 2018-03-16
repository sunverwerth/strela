#ifndef Strela_Ast_AstIdTypeExpr_h
#define Strela_Ast_AstIdTypeExpr_h

#include "TypeExpr.h"

#include <string>

namespace Strela {
    class Symbol;

    class IdTypeExpr: public TypeExpr {
    public:
        IdTypeExpr(const Token& startToken, const std::string& name): TypeExpr(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::IdTypeExpr, Strela::TypeExpr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif