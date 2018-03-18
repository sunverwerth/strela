#ifndef Strela_Ast_AstTypeExpr_h
#define Strela_Ast_AstTypeExpr_h

#include "Node.h"
#include "../ITypeExprVisitor.h"

#define STRELA_IMPL_TYPE_EXPR_VISITOR void accept(Strela::ITypeExprVisitor& v) override { v.visit(*this); }

namespace Strela {
    class TypeDecl;
    
    class TypeExpr: public Node {
    public:
        TypeExpr(): Node() {}
        STRELA_GET_TYPE(Strela::TypeExpr, Strela::Node);
        virtual void accept(ITypeExprVisitor&) = 0;

    public:
        TypeDecl* type = nullptr;
    };
}

#endif