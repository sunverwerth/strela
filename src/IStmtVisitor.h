// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_IStmtVisitor_h
#define Strela_IStmtVisitor_h

namespace Strela {
    class IStmtVisitor {
    public:
        virtual void visit(class VarDecl&) = 0;
        virtual void visit(class RetStmt&) = 0;
        virtual void visit(class BlockStmt&) = 0;
        virtual void visit(class ExprStmt&) = 0;
        virtual void visit(class IfStmt&) = 0;
        virtual void visit(class WhileStmt&) = 0;
    };
}

#endif