#ifndef Strela_Ast_AstFieldDecl_h
#define Strela_Ast_AstFieldDecl_h

#include "Stmt.h"

#include <string>

namespace Strela {
    class TypeExpr;
    class Type;
    
    class FieldDecl: public Stmt {
    public:
        FieldDecl(const Token& startToken, const std::string& name, TypeExpr* typeExpr): Stmt(startToken), name(name), typeExpr(typeExpr) {}
        STRELA_GET_TYPE(Strela::FieldDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        TypeExpr* typeExpr;

        int index = 0;
        Type* declType = nullptr;
    };
}

#endif