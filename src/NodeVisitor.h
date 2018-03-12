#ifndef Strela_NodeVisitor_h
#define Strela_NodeVisitor_h

namespace Strela {
    class AstModDecl;
    class AstFuncDecl;
    class AstParam;
    class AstVarDecl;
    class AstIdExpr;
    class AstRetStmt;
    class AstLitExpr;
    class AstNode;
    class AstBlockStmt;
    class AstCallExpr;
    class AstExprStmt;
    class AstBinopExpr;
    class AstClassDecl;
    class AstScopeExpr;
    class AstIfStmt;
    class AstFieldDecl;
    class AstNewExpr;
    class AstAssignExpr;
    class AstIdTypeExpr;
    class AstPostfixExpr;
    class AstWhileStmt;
    class AstExpr;
    class AstArrayTypeExpr;
    class AstImportStmt;
    class AstUnaryExpr;
    class AstEnumDecl;
    class AstEnumElement;

    class NodeVisitor {
    public:
        virtual void visit(AstModDecl&) = 0;
        virtual void visit(AstImportStmt&) = 0;
        virtual void visit(AstFuncDecl&) = 0;
        virtual void visit(AstParam&) = 0;
        virtual void visit(AstVarDecl&) = 0;
        virtual void visit(AstIdExpr&) = 0;
        virtual void visit(AstClassDecl&) = 0;
        virtual void visit(AstFieldDecl&) = 0;
        virtual void visit(AstEnumDecl&) = 0;
        virtual void visit(AstEnumElement&) = 0;

        virtual void visit(AstRetStmt&) = 0;
        virtual void visit(AstBlockStmt&) = 0;
        virtual void visit(AstExprStmt&) = 0;
        virtual void visit(AstIfStmt&) = 0;
        virtual void visit(AstWhileStmt&) = 0;

        virtual void visit(AstLitExpr&) = 0;
        virtual void visit(AstCallExpr&) = 0;
        virtual void visit(AstBinopExpr&) = 0;
        virtual void visit(AstScopeExpr&) = 0;
        virtual void visit(AstNewExpr&) = 0;
        virtual void visit(AstAssignExpr&) = 0;
        virtual void visit(AstPostfixExpr&) = 0;
        virtual void visit(AstUnaryExpr&) = 0;

        virtual void visit(AstIdTypeExpr&) = 0;
        virtual void visit(AstArrayTypeExpr&) = 0;

        template<typename T> void visitChildren(T& children) {
            for (auto&& child: children) {
                child->accept(*this);
            }
        }

        void visitChild(AstNode* child);
    };
}

#endif