#ifndef Strela_Ast_AstLitExpr_h
#define Strela_Ast_AstLitExpr_h

#include "Expr.h"
#include "Token.h"

namespace Strela {
    class LitExpr: public Expr {
    public:
        LitExpr(const Token& token): Expr(token), token(token) {}
        STRELA_GET_TYPE(Strela::LitExpr, Strela::Expr);
        STRELA_IMPL_EXPR_VISITOR;

    public:
        Token token;
    };
}

#endif