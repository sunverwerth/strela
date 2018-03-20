#ifndef Strela_ITypeExprVisitor_h
#define Strela_ITypeExprVisitor_h

namespace Strela {
    class IdTypeExpr;
    class ArrayTypeExpr;
    class UnionTypeExpr;

    class ITypeExprVisitor {
    public:
        virtual void visit(IdTypeExpr&) = 0;
        virtual void visit(ArrayTypeExpr&) = 0;
        virtual void visit(UnionTypeExpr&) = 0;
    };
}

#endif