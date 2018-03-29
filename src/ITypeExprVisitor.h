// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_ITypeExprVisitor_h
#define Strela_ITypeExprVisitor_h

namespace Strela {
    class ITypeExprVisitor {
    public:
        virtual void visit(class IdTypeExpr&) = 0;
        virtual void visit(class ArrayTypeExpr&) = 0;
        virtual void visit(class UnionTypeExpr&) = 0;
        virtual void visit(class NullableTypeExpr&) = 0;
        virtual void visit(class GenericReificationExpr&) = 0;
    };
}

#endif