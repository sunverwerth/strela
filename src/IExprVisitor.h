// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_IExprVisitor_h
#define Strela_IExprVisitor_h

namespace Strela {
    template<typename T> class IExprVisitor {
    public:
        virtual T visit(class IdExpr&) = 0;
        virtual T visit(class LitExpr&) = 0;
        virtual T visit(class CallExpr&) = 0;
        virtual T visit(class BinopExpr&) = 0;
        virtual T visit(class ScopeExpr&) = 0;
        virtual T visit(class NewExpr&) = 0;
        virtual T visit(class AssignExpr&) = 0;
        virtual T visit(class PostfixExpr&) = 0;
        virtual T visit(class UnaryExpr&) = 0;
        virtual T visit(class ThisExpr&) = 0;
        virtual T visit(class CastExpr&) = 0;
        virtual T visit(class IsExpr&) = 0;
        virtual T visit(class ArrayLitExpr&) = 0;
        virtual T visit(class SubscriptExpr&) = 0;
        virtual T visit(class ArrayTypeExpr&) = 0;
        virtual T visit(class NullableTypeExpr&) = 0;
        virtual T visit(class UnionTypeExpr&) = 0;
        virtual T visit(class GenericReificationExpr&) = 0;
    };
}

#endif