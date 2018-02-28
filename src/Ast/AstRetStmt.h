#ifndef Strela_Ast_AstRetStmt_h
#define Strela_Ast_AstRetStmt_h

#include "AstStmt.h"

namespace Strela {
    class AstExpr;

    class AstRetStmt: public AstStmt {
    public:
        AstRetStmt(const Token& startToken, AstExpr* expression): AstStmt(startToken), expression(expression) {}
        STRELA_GET_TYPE(Strela::AstRetStmt, Strela::AstStmt);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* expression;
    };
}

#endif