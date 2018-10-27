// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstFuncDecl_h
#define Strela_Ast_AstFuncDecl_h

#include "Node.h"

#include <string>
#include <vector>

namespace Strela {
    class Param;
    class Expr;
    class FuncType;
    class Stmt;
	class VM;

	typedef void(*BuiltinFunction)(VM&);

    class FuncDecl: public Node {
    public:
        STRELA_GET_TYPE(Strela::FuncDecl, Strela::Node);

    public:
        bool returns = false;
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
		BuiltinFunction builtin = nullptr;
    };
}

#endif