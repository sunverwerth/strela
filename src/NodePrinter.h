#ifndef Strela_NodePrinter_h
#define Strela_NodePrinter_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "ITypeExprVisitor.h"

#include <string>
#include <iostream>

namespace Strela {
    class NodePrinter: public IStmtVisitor, public IExprVisitor, public ITypeExprVisitor {
    public:
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
        void visit(IdTypeExpr&) override;
        void visit(WhileStmt&) override;
        void visit(PostfixExpr&) override;
        void visit(ArrayTypeExpr&) override;
        void visit(ImportStmt&) override;
        void visit(UnaryExpr&) override;
        void visit(EnumDecl&) override;
        void visit(EnumElement&) override;
        void visit(InterfaceDecl&) override;
        void visit(InterfaceMethodDecl&) override;
        void visit(ThisExpr&) override;
        void visit(CastExpr&) override;

        void push();
        void pop();

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
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
        int indentation = 0;
        std::string indent;
    };
}
#endif