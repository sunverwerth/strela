// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_ITypeExprVisitor_h
#define Strela_ITypeExprVisitor_h

namespace Strela {
    class IdTypeExpr;
    class ArrayTypeExpr;
    class UnionTypeExpr;
    class NullableTypeExpr;
    class GenericReificationExpr;

    class ITypeExprVisitor {
    public:
        virtual void visit(IdTypeExpr&) = 0;
        virtual void visit(ArrayTypeExpr&) = 0;
        virtual void visit(UnionTypeExpr&) = 0;
        virtual void visit(NullableTypeExpr&) = 0;
        virtual void visit(GenericReificationExpr&) = 0;
    };
}

#endif