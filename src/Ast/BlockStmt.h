#ifndef Strela_Ast_AstBlockStmt_h
#define Strela_Ast_AstBlockStmt_h

#include "Stmt.h"

#include <vector>

namespace Strela {
    class BlockStmt: public Stmt {
    public:
        BlockStmt(const std::vector<Stmt*>& stmts): Stmt(), stmts(stmts) {}
        STRELA_GET_TYPE(Strela::BlockStmt, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::vector<Stmt*> stmts;
    };
}
#endif