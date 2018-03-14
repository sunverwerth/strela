#ifndef Strela_IExprVisitor_h
#define Strela_IExprVisitor_h

namespace Strela {
    class AstIdExpr;
    class AstLitExpr;
    class AstCallExpr;
    class AstBinopExpr;
    class AstScopeExpr;
    class AstNewExpr;
    class AstAssignExpr;
    class AstIdTypeExpr;
    class AstPostfixExpr;
    class AstArrayTypeExpr;
    class AstUnaryExpr;

    class IExprVisitor {
    public:
        virtual void visit(AstIdExpr&) = 0;
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
    };
}

#endif