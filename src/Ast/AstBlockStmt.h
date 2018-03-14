#ifndef Strela_Ast_AstBlockStmt_h
#define Strela_Ast_AstBlockStmt_h

#include "AstStmt.h"

#include <vector>

namespace Strela {
    class AstBlockStmt: public AstStmt {
    public:
        AstBlockStmt(const Token& startToken, const std::vector<AstStmt*>& stmts): AstStmt(startToken), stmts(stmts) {}
        STRELA_GET_TYPE(Strela::AstBlockStmt, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::vector<AstStmt*> stmts;
    };
}
#endif