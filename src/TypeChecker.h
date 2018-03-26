// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_TypeChecker_h
#define Strela_TypeChecker_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "Pass.h"

#include <string>
#include <vector>
#include <map>

namespace Strela {
    class Refinement;
    class TypeDecl;
    class Node;
    class Stmt;
    class Expr;

    class TypeChecker: public Pass, public IStmtVisitor, public IExprVisitor {
    public:
        TypeChecker();

        void visit(ModDecl&) override;
        void visit(FuncDecl&) override;
        void visit(VarDecl&) override;
        void visit(RetStmt&) override;
        void visit(LitExpr&) override;
        void visit(BlockStmt&) override;
        void visit(CallExpr&) override;
        void visit(ExprStmt&) override;
        void visit(BinopExpr&) override;
        void visit(ClassDecl&) override;
        void visit(ScopeExpr&) override;
        void visit(IfStmt&) override;
        void visit(NewExpr&) override;
        void visit(AssignExpr&) override;
        void visit(WhileStmt&) override;
        void visit(PostfixExpr&) override;
        void visit(UnaryExpr&) override;
        void visit(InterfaceDecl&) override;
        void visit(InterfaceMethodDecl&) override;
        void visit(ThisExpr&) override;
        void visit(CastExpr&) override {};
        void visit(GenericParam&) override {}

        void visit(IdExpr&) override;
        void visit(FieldDecl&) override {}
        void visit(ImportStmt&) override {}
        void visit(EnumDecl&) override {}
        void visit(EnumElement&) override {}
        void visit(Param&) override {}
        void visit(IsExpr&) override;
        void visit(ArrayLitExpr&) override;
        void visit(SubscriptExpr&) override;

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
        }

        void negateRefinements();
        
        std::vector<TypeDecl*> getTypes(const std::vector<Expr*>& arguments);
        TypeDecl* getType(Expr* expr);

    private:
		std::vector<FuncDecl*> findOverload(Expr* target, const std::vector<TypeDecl*>& argtypes);
		std::vector<FuncDecl*> findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<Expr*>& args);
		std::vector<FuncDecl*> findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<TypeDecl*>& argTypes);

        FuncDecl* function = nullptr;
        BlockStmt* block = nullptr;
        ClassDecl* _class = nullptr;

        bool returns = false;

        std::vector<ClassDecl*> genericStack;

        std::map<Node*, std::vector<Refinement>> refinements;
    };
}
#endif