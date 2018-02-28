#ifndef Strela_Ast_AstWhileStmt_h
#define Strela_Ast_AstWhileStmt_h

#include "AstStmt.h"

namespace Strela {
    class AstExpr;

    class AstWhileStmt: public AstStmt {
    public:
        AstWhileStmt(const Token& startToken, AstExpr* condition, AstStmt* body): AstStmt(startToken), condition(condition), body(body) {}
        STRELA_GET_TYPE(Strela::AstWhileStmt, Strela::AstStmt);
        STRELA_IMPL_VISITOR;

    public:
        AstExpr* condition;
        AstStmt* body;
    };
}

#endif