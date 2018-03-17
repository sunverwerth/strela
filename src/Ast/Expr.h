#ifndef Strela_Ast_AstExpr_h
#define Strela_Ast_AstExpr_h

#include "Node.h"
#include "../IExprVisitor.h"

#include <vector>

#define STRELA_IMPL_EXPR_VISITOR void accept(Strela::IExprVisitor& v) override { v.visit(*this); }

namespace Strela {
    class TypeDecl;
    class FuncDecl;

    class Expr: public Node {
    public:
        Expr(const Token& startToken): Node(startToken) {}
        STRELA_GET_TYPE(Strela::Expr, Strela::Node);
        virtual void accept(IExprVisitor&) = 0;

    public:
        TypeDecl* type = nullptr;
        Node* node = nullptr;
        std::vector<FuncDecl*> candidates;
    };
}

#endif