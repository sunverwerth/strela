// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_IExprVisitor_h
#define Strela_IExprVisitor_h

namespace Strela {
    class IExprVisitor {
    public:
        virtual void visit(class IdExpr&) = 0;
        virtual void visit(class LitExpr&) = 0;
        virtual void visit(class CallExpr&) = 0;
        virtual void visit(class BinopExpr&) = 0;
        virtual void visit(class ScopeExpr&) = 0;
        virtual void visit(class NewExpr&) = 0;
        virtual void visit(class AssignExpr&) = 0;
        virtual void visit(class PostfixExpr&) = 0;
        virtual void visit(class UnaryExpr&) = 0;
        virtual void visit(class ThisExpr&) = 0;
        virtual void visit(class CastExpr&) = 0;
        virtual void visit(class IsExpr&) = 0;
        virtual void visit(class ArrayLitExpr&) = 0;
        virtual void visit(class SubscriptExpr&) = 0;
    };
}

#endif