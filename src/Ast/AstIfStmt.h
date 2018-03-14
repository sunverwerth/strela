#ifndef Strela_Ast_AstIfStmt_h
#define Strela_Ast_AstIfStmt_h

#include "AstStmt.h"

namespace Strela {
    class AstExpr;
    class IStmtVisitor;

    class AstIfStmt: public AstStmt {
    public:
        AstIfStmt(
            const Token& startToken,
            AstExpr* condition,
            AstStmt* trueBranch,
            AstStmt* falseBranch
        ): AstStmt(startToken), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch) {}
        STRELA_GET_TYPE(Strela::AstIfStmt, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        AstExpr* condition;
        AstStmt* trueBranch;
        AstStmt* falseBranch;
    };
}

#endif