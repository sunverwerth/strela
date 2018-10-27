// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstInterfaceMethodDecl_h
#define Strela_Ast_AstInterfaceMethodDecl_h

#include "Node.h"

#include <string>
#include <vector>

namespace Strela {
    class BlockStmt;
    class Param;
    class Expr;
    class ByteCodeChunk;
    class FuncType;
    class TypeDecl;

    class InterfaceMethodDecl: public Node {
    public:
        STRELA_GET_TYPE(Strela::InterfaceMethodDecl, Strela::Node);
        std::string getDescription();

    public:
        bool isExported = false;
        std::string name;
        std::vector<Param*> params;
        Expr* returnTypeExpr = nullptr;
        FuncType* type = nullptr;
        size_t index = 0;
    };
}

#endif