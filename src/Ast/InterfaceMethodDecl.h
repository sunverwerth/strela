#ifndef Strela_Ast_AstInterfaceMethodDecl_h
#define Strela_Ast_AstInterfaceMethodDecl_h

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

    class InterfaceMethodDecl: public Stmt {
    public:
        InterfaceMethodDecl(const std::string& name, const std::vector<Param*>& params, TypeExpr* returnTypeExpr): Stmt(), name(name), params(params), returnTypeExpr(returnTypeExpr) {}
        STRELA_GET_TYPE(Strela::InterfaceMethodDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        bool isExported = false;
        std::string name;
        std::vector<Param*> params;
        TypeExpr* returnTypeExpr;
        FuncType* type = nullptr;
        size_t index = 0;
    };
}

#endif