#ifndef Strela_Ast_AstBlockStmt_h
#define Strela_Ast_AstBlockStmt_h

#include "Stmt.h"

#include <vector>

namespace Strela {
    class BlockStmt: public Stmt {
    public:
        BlockStmt(const Token& startToken, const std::vector<Stmt*>& stmts): Stmt(startToken), stmts(stmts) {}
        STRELA_GET_TYPE(Strela::BlockStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::vector<Stmt*> stmts;
    };
}
#endif