#ifndef Strela_Ast_AstNewExpr_h
#define Strela_Ast_AstNewExpr_h

#include "Expr.h"

#include <string>
#include <vector>

namespace Strela {
    class TypeExpr;

    class NewExpr: public Expr {
    public:
        NewExpr(TypeExpr* typeExpr, const std::vector<Expr*>& arguments): Expr(), typeExpr(typeExpr), arguments(arguments) {}
        STRELA_GET_TYPE(Strela::NewExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        TypeExpr* typeExpr;
        std::vector<Expr*> arguments;
        FuncDecl* initMethod = nullptr;
    };
}

#endif