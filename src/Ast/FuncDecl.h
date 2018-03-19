#ifndef Strela_Ast_AstFuncDecl_h
#define Strela_Ast_AstFuncDecl_h

#include "Stmt.h"

#include <string>
#include <vector>

namespace Strela {
    class BlockStmt;
    class Param;
    class Expr;
    class TypeExpr;
    class ByteCodeChunk;
    class FuncType;
    class TypeDecl;

    class FuncDecl: public Stmt {
    public:
        FuncDecl(const std::string& name, const std::vector<Param*>& params, TypeExpr* returnTypeExpr, const std::vector<Stmt*>& stmts): Stmt(), name(name), params(params), returnTypeExpr(returnTypeExpr), stmts(stmts) {}
        STRELA_GET_TYPE(Strela::FuncDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        bool isExported = false;
        std::string name;
        std::vector<Param*> params;
        TypeExpr* returnTypeExpr;
        FuncType* type = nullptr;
        std::vector<Stmt*> stmts;
        size_t opcodeStart = 0xdeadbeef;
        int numVariables = 0;
    };
}

#endif