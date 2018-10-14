// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_NodePrinter_h
#define Strela_NodePrinter_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"

#include <string>
#include <iostream>

namespace Strela {
    class ModDecl;
    class ClassDecl;
    class FuncDecl;
    class EnumDecl;
    class EnumElement;
    class InterfaceDecl;
    class InterfaceMethodDecl;
    class TypeAliasDecl;
    class ImportStmt;
    class Param;
    class FieldDecl;
    class GenericParam;

    class NodePrinter: public IStmtVisitor, public IExprVisitor {
    public:
        void print(ModDecl&);
        void print(ClassDecl&);
        void print(FuncDecl&);
        void print(EnumDecl&);
        void print(EnumElement&);
        void print(InterfaceDecl&);
        void print(InterfaceMethodDecl&);
        void print(TypeAliasDecl&);
        void print(ImportStmt&);
        void print(Param&);
        void print(FieldDecl&);
        void print(GenericParam&);
        
        void visit(VarDecl&) override;
        void visit(IdExpr&) override;
        void visit(RetStmt&) override;
        void visit(LitExpr&) override;
        void visit(BlockStmt&) override;
        void visit(CallExpr&) override;
        void visit(ExprStmt&) override;
        void visit(BinopExpr&) override;
        void visit(ScopeExpr&) override;
        void visit(IfStmt&) override;
        void visit(NewExpr&) override;
        void visit(AssignExpr&) override;
        void visit(WhileStmt&) override;
        void visit(PostfixExpr&) override;
        void visit(ArrayTypeExpr&) override;
        void visit(UnaryExpr&) override;
        void visit(ThisExpr&) override;
        void visit(CastExpr&) override;
        void visit(IsExpr&) override;
        void visit(UnionTypeExpr&) override;
        void visit(ArrayLitExpr&) override;
        void visit(SubscriptExpr&) override;
        void visit(MapLitExpr&) override;
        void visit(NullableTypeExpr&) override;
        void visit(GenericReificationExpr&) override;

        void push();
        void pop();

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            if (child) child->accept(*this);
        }

        template<typename T> void list(const T& children, const std::string& separator) {
            for (auto&& child: children) {
                child->accept(*this);
                if (&child != &children.back()) {
                    std::cout << separator;
                }
            }
        }

    private:
        void indent();
        int indentation = 0;
        std::string indentstring;
    };
}
#endif