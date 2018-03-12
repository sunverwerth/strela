#ifndef Strela_NodePrinter_h
#define Strela_NodePrinter_h

#include "NodeVisitor.h"

#include <string>

namespace Strela {
    class NodePrinter: public NodeVisitor {
    public:
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

        void push();
        void pop();

    private:
        int indentation = 0;
        std::string indent;
    };
}
#endif