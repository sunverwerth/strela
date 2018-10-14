// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_NameResolver_h
#define Strela_NameResolver_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "Pass.h"

#include <string>

namespace Strela {
    class Scope;
    class Node;
    class ModDecl;
    class ClassDecl;
    class FuncDecl;
    class Param;
    class FieldDecl;
    class ImportStmt;
    class InterfaceDecl;
    class InterfaceMethodDecl;
    class InterfaceFieldDecl;
    class TypeAliasDecl;

    class NameResolver: public Pass, public IStmtVisitor, public IExprVisitor {
    public:
        NameResolver(Scope* globals);
        void resolve(ModDecl&);
        void resolve(ClassDecl&);
        void resolve(FuncDecl&);
        void resolve(Param&);
        void resolve(ImportStmt&);
        void resolve(InterfaceDecl&);
        void resolve(InterfaceMethodDecl&);
        void resolve(InterfaceFieldDecl&);
        void resolve(TypeAliasDecl&);
        void resolve(FieldDecl&);

        void visit(BlockStmt&) override;
        void visit(ExprStmt&) override;
        void visit(IfStmt&) override;
        void visit(RetStmt&) override;
        void visit(VarDecl&) override;
        void visit(WhileStmt&) override;

        void visit(ArrayLitExpr&) override;
        void visit(ArrayTypeExpr&) override;
        void visit(AssignExpr&) override;
        void visit(BinopExpr&) override;
        void visit(CallExpr&) override;
        void visit(CastExpr&) override;
        void visit(GenericReificationExpr&) override;
        void visit(IdExpr&) override;
        void visit(IsExpr&) override;
        void visit(LitExpr&) override {}
        void visit(MapLitExpr&) override;
        void visit(NewExpr&) override;
        void visit(NullableTypeExpr&) override;
        void visit(PostfixExpr&) override;
        void visit(ScopeExpr&) override;
        void visit(SubscriptExpr&) override;
        void visit(ThisExpr&) override {};
        void visit(UnaryExpr&) override;
        void visit(UnionTypeExpr&) override;

        int resolveGenerics(ModDecl&);

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                visitChild(child);
            }
        }

        template<typename T> void visitChild(T& child) {
            if (child) {
                child->accept(*this);
            }
        }
        
    private:
        Scope* scope;
    };
}
#endif