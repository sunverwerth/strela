#ifndef Strela_NameResolver_h
#define Strela_NameResolver_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"
#include "ITypeExprVisitor.h"

#include <string>

namespace Strela {
    class Scope;
    class Node;

    class NameResolver: public IStmtVisitor, public IExprVisitor, public ITypeExprVisitor {
    public:
        NameResolver(Scope* globals);

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
        void visit(ThisExpr&) override {};
        void visit(CastExpr&) override {};
        void visit(IsExpr&) override;
        void visit(UnionTypeExpr&) override;
        void visit(ArrayLitExpr&) override;
        void visit(SubscriptExpr&) override;
        void visit(NullableTypeExpr&) override;
        void visit(GenericParam&) override;
        void visit(GenericReificationExpr&) override;

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
        }

        void error(const Node& n, const std::string& msg);

    private:
        ClassDecl* _class = nullptr;
        InterfaceDecl* _interface = nullptr;
        Scope* scope;
    };
}
#endif