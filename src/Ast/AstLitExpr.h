#ifndef Strela_Ast_AstLitExpr_h
#define Strela_Ast_AstLitExpr_h

#include "AstExpr.h"
#include "Token.h"

namespace Strela {
    class AstLitExpr: public AstExpr {
    public:
        AstLitExpr(const Token& token): AstExpr(token), token(token) {}
        STRELA_GET_TYPE(Strela::AstLitExpr, Strela::AstExpr);
        STRELA_IMPL_VISITOR;

    public:
        Token token;
    };
}

#endif