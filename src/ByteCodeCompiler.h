// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_ByteCodeCompiler_h
#define Strela_ByteCodeCompiler_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "Pass.h"

#include <string>
#include <map>
#include <vector>

namespace Strela {
    class ByteCodeChunk;
    class Expr;
    class Node;
    class VMType;
    class TypeDecl;
    class FuncDecl;
    class ClassDecl;
    class ModDecl;
    class FieldDecl;
    class Param;

    class ByteCodeCompiler: public Pass, public IStmtVisitor, public IExprVisitor {
    public:
        ByteCodeCompiler(ByteCodeChunk&);
        void compile(ModDecl&);
        void compile(ClassDecl&);
        void compile(FuncDecl&);
        void compile(FieldDecl&);
        void compile(Param&);

        void visit(BlockStmt&) override;
        void visit(ExprStmt&) override;
        void visit(IfStmt&) override;
        void visit(RetStmt&) override;
        void visit(VarDecl&) override;
        void visit(WhileStmt&) override;

        void visit(ArrayLitExpr&) override;
        void visit(ArrayTypeExpr&) override {}
        void visit(AssignExpr&) override;
        void visit(BinopExpr&) override;
        void visit(CallExpr&) override;
        void visit(CastExpr&) override;
        void visit(GenericReificationExpr&) override {}
        void visit(IdExpr&) override;
        void visit(IsExpr&) override;
        void visit(LitExpr&) override;
        void visit(MapLitExpr&) override;
        void visit(NewExpr&) override;
        void visit(NullableTypeExpr&) override {}
        void visit(PostfixExpr&) override;
        void visit(ScopeExpr&) override;
        void visit(SubscriptExpr&) override;
        void visit(ThisExpr&) override;
        void visit(UnaryExpr&) override;
        void visit(UnionTypeExpr&) override {}

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            if (child) child->accept(*this);
        }

    private:
        void addFixup(size_t address, FuncDecl* function, bool immediate);
        VMType* mapType(TypeDecl* type);

    private:
        struct Fixup {
            size_t address;
            FuncDecl* function;
            bool immediate;
        };
        std::vector<Fixup> functionFixups;
        FuncDecl* function = nullptr;
        std::map<TypeDecl*, VMType*> typeMap;

    public:
        ByteCodeChunk& chunk;
        ClassDecl* _class = nullptr;
    };
}
#endif