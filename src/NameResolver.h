#ifndef Strela_NameResolver_h
#define Strela_NameResolver_h

#include "IStmtVisitor.h"
#include "IExprVisitor.h"


namespace Strela {
    class Scope;

    class NameResolver: public IStmtVisitor, public IExprVisitor {
    public:
        NameResolver(Scope* globals);

        void visit(AstModDecl&) override;
        void visit(AstFuncDecl&) override;
        void visit(AstParam&) override;
        void visit(AstVarDecl&) override;
        void visit(AstIdExpr&) override;
        void visit(AstRetStmt&) override;
        void visit(AstLitExpr&) override;
        void visit(AstBlockStmt&) override;
        void visit(AstCallExpr&) override;
        void visit(AstExprStmt&) override;
        void visit(AstBinopExpr&) override;
        void visit(AstClassDecl&) override;
        void visit(AstScopeExpr&) override;
        void visit(AstIfStmt&) override;
        void visit(AstFieldDecl&) override;
        void visit(AstNewExpr&) override;
        void visit(AstAssignExpr&) override;
        void visit(AstIdTypeExpr&) override;
        void visit(AstWhileStmt&) override;
        void visit(AstPostfixExpr&) override;
        void visit(AstArrayTypeExpr&) override;
        void visit(AstImportStmt&) override;
        void visit(AstUnaryExpr&) override;
        void visit(AstEnumDecl&) override;
        void visit(AstEnumElement&) override;

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        template<typename T> void visitChild(T& child) {
            child->accept(*this);
        }

    private:
        Scope* scope;
    };
}
#endif