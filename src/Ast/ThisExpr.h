#ifndef Strela_Ast_AstThisExpr_h
#define Strela_Ast_AstThisExpr_h

#include "Expr.h"

namespace Strela {
    class ThisExpr: public Expr {
    public:
        ThisExpr(): Expr() {}
        STRELA_GET_TYPE(Strela::ThisExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        ClassDecl* _class = nullptr;
    };
}

#endif