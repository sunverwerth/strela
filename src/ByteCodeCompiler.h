// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_ByteCodeCompiler_h
#define Strela_ByteCodeCompiler_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"

#include <string>
#include <map>

namespace Strela {
    class ByteCodeChunk;
    class Expr;
    class Node;

    class ByteCodeCompiler: public IStmtVisitor, public IExprVisitor {
    public:
        ByteCodeCompiler(ByteCodeChunk&);

        void visit(ModDecl&) override;
        void visit(FuncDecl&) override;
        void visit(Param&) override;
        void visit(VarDecl&) override;
        void visit(IdExpr&) override;
        void visit(RetStmt&) override;
        void visit(LitExpr&) override;
        void visit(BlockStmt&) override;
        void visit(CallExpr&) override;
        void visit(ExprStmt&) override;
        void visit(BinopExpr&) override;
        void visit(ClassDecl&) override;
        void visit(ScopeExpr&) override;
        void visit(IfStmt&) override;
        void visit(FieldDecl&) override;
        void visit(NewExpr&) override;
        void visit(AssignExpr&) override;
        void visit(WhileStmt&) override;
        void visit(PostfixExpr&) override;
        void visit(ImportStmt&) override {}
        void visit(UnaryExpr&) override;
        void visit(EnumDecl&) override;
        void visit(EnumElement&) override;
        void visit(InterfaceDecl&) {}
        void visit(InterfaceMethodDecl&) {}
        void visit(ThisExpr&) override;
        void visit(CastExpr&) override;
        void visit(IsExpr&) override;
        void visit(ArrayLitExpr&) override;
        void visit(SubscriptExpr&) override;
        void visit(GenericParam&) {}

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
        }

    private:
        void error(Node& node, const std::string& msg);
        void addFixup(int address, FuncDecl* function);

    private:
        std::map<int, FuncDecl*> functionFixups;
        FuncDecl* function = nullptr;

    public:
        ByteCodeChunk& chunk;
        ClassDecl* _class = nullptr;
    };
}
#endif