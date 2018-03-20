#ifndef Strela_Ast_AstExpr_h
#define Strela_Ast_AstExpr_h

#include "Node.h"
#include "../IExprVisitor.h"

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
        Refinement(Node* node): node(node) {}
        Node* node;
    };

    class TypeRefinement: public Refinement {
    public:
        TypeRefinement(Node* node, TypeDecl* type): Refinement(node), type(type) {}
        TypeDecl* type;
    };

    class RangeRefinement: public Refinement {
    public:
        int lower;
        int upper;
    };

    class Expr: public Node {
    public:
        Expr(): Node() {}
        STRELA_GET_TYPE(Strela::Expr, Strela::Node);
        virtual void accept(IExprVisitor&) = 0;

    public:
        TypeDecl* type = nullptr;
        Node* node = nullptr;
        Expr* context = nullptr;
        std::vector<FuncDecl*> candidates;
        std::vector<Refinement*> refinements;
    };
}

#endif