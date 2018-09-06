// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstFuncDecl_h
#define Strela_Ast_AstFuncDecl_h

#include "Stmt.h"

#include <string>
#include <vector>

namespace Strela {
    class BlockStmt;
    class Param;
    class Expr;
    class ByteCodeChunk;
    class FuncType;
    class TypeDecl;

    class FuncDecl: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::FuncDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        bool isExported = false;
        std::string name;
        std::vector<Param*> params;
        Expr* returnTypeExpr = nullptr;
        FuncType* declType = nullptr;
        std::vector<Stmt*> stmts;
        size_t opcodeStart = 0xdeadbeef;
        int numVariables = 0;
        bool isPrototype = false;
        bool isExternal = false;
    };
}

#endif