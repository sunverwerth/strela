// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_TypeChecker_h
#define Strela_TypeChecker_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "Pass.h"
#include "Ast/TypeDecl.h"
#include "Ast/InvalidType.h"

#include <string>
#include <vector>
#include <map>

namespace Strela {
    class Refinement;
    class TypeDecl;
    class Node;
    class Stmt;
    class Expr;
    class Param;
    class FuncDecl;
    class ClassDecl;
    class ModDecl;
    class InterfaceDecl;

    class TypeChecker: public Pass, public IStmtVisitor, public IExprVisitor {
    public:
        TypeChecker();
        void check(ModDecl&);
        void check(ClassDecl&);
        void check(FuncDecl&);

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

        template<typename T> bool unify(T&& child, TypeDecl* with) {
            auto oldExpectedType = expectedType;
            expectedType = with;
            bool success = true;
            if (child) {
                child->accept(*this);
                if (expectedType && !isAssignableFrom(expectedType, getType(child))) {
                    if (child->type != &InvalidType::instance) {
                        error(*child, child->type->getFullName() + " is not coercible to " + expectedType->getFullName());
                        success = false;
                    }
                }
                else if (expectedType) {
                    child = addCast(child, expectedType);
                }
            }
            expectedType = oldExpectedType;
            return success;
        }

        template<typename T> bool unifyChildren(T& children, TypeDecl* with) {
            auto oldExpectedType = expectedType;
            expectedType = with;
            bool success = true;
            for (auto&& child: children) {
                if (child) {
                    child->accept(*this);
                    if (expectedType && !isAssignableFrom(expectedType, getType(child))) {
                        if (child->type != &InvalidType::instance) {
                            error(*child, child->type->getFullName() + " is not coercible to " + expectedType->getFullName());
                            success = false;
                        }
                    }
                    else if (expectedType) {
                        child = addCast(child, expectedType);
                    }
                }
            }
            expectedType = oldExpectedType;
            return success;
        }

        void negateRefinements();
        
        std::vector<TypeDecl*> getTypes(const std::vector<Expr*>& arguments);
        TypeDecl* getType(Expr* expr);
        TypeDecl* getType(VarDecl* var);
        TypeDecl* getType(Param* param);

    private:
		std::vector<FuncDecl*> findOverload(Expr* target, const std::vector<TypeDecl*>& argtypes);
		std::vector<FuncDecl*> findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<Expr*>& args);
		std::vector<FuncDecl*> findOverload(const std::vector<FuncDecl*>& funcs, const std::vector<TypeDecl*>& argTypes);

        void printMissingMembers(ClassDecl* cls, InterfaceDecl* iface);

        FuncDecl* function = nullptr;
        BlockStmt* block = nullptr;
        ClassDecl* _class = nullptr;
        TypeDecl* expectedType = nullptr;

        bool returns = false;

        std::vector<std::map<Node*, std::vector<Refinement>>> refinements;
    };
}
#endif