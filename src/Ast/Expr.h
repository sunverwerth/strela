// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstExpr_h
#define Strela_Ast_AstExpr_h

#include "Node.h"
#include "../IExprVisitor.h"
#include "InvalidType.h"
#include <vector>

#define STRELA_IMPL_EXPR_VISITOR void accept(Strela::IExprVisitor& v) override { v.visit(*this); }

namespace Strela {
    class TypeDecl;
    class FuncDecl;
    class Param;
    class FieldDecl;
    class VarDecl;
    class ClassDecl;
    class EnumDecl;
    class EnumElement;
    class ModDecl;

    class Refinement {
    public:
        Node* node;
        TypeDecl* type;
    };

    class Expr: public Node {
    public:
        STRELA_GET_TYPE(Strela::Expr, Strela::Node);
        virtual void accept(IExprVisitor&) = 0;

    public:
        TypeDecl* type = &InvalidType::instance;
        TypeDecl* typeValue = &InvalidType::instance;
        Node* node = nullptr;
        Expr* arrayIndex = nullptr;
        Expr* context = nullptr;
        std::vector<FuncDecl*> candidates;
        std::vector<Refinement*> refinements;
    };
}

#endif