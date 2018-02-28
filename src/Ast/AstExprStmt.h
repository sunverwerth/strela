#ifndef Strela_Ast_AstExprStmt_h
#define Strela_Ast_AstExprStmt_h

#include "AstStmt.h"

namespace Strela {
    class AstExpr;

    class AstExprStmt: public AstStmt {
    public:
        AstExprStmt(const Token& startToken, AstExpr* expression): AstStmt(startToken), expression(expression) {}
        STRELA_GET_TYPE(Strela::AstExprStmt, Strela::AstStmt);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* expression;
    };
}

#endif