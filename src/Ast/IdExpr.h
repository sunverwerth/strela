#ifndef Strela_Ast_AstIdExpr_h
#define Strela_Ast_AstIdExpr_h

#include "Expr.h"

#include <string>

namespace Strela {
    class IdExpr: public Expr {
    public:
        IdExpr(const std::string& name): Expr(), name(name) {}
        STRELA_GET_TYPE(Strela::IdExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        std::string name;
    };
}

#endif