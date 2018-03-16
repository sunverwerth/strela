#ifndef Strela_Ast_AstIdExpr_h
#define Strela_Ast_AstIdExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class Symbol;

    class IdExpr: public Expr {
    public:
        IdExpr(const Token& startToken, const std::string& name): Expr(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::IdExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::string name;
        Symbol* symbol = nullptr;
    };
}

#endif