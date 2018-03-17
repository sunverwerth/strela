#ifndef Strela_Ast_AstFieldDecl_h
#define Strela_Ast_AstFieldDecl_h

#include "Stmt.h"

#include <string>

namespace Strela {
    class TypeDecl;
    class TypeExpr;
    
    class FieldDecl: public Stmt {
    public:
        FieldDecl(const Token& startToken, const std::string& name, TypeExpr* typeExpr): Stmt(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::FieldDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        TypeExpr* typeExpr;
        TypeDecl* type = nullptr;

        int index = 0;
    };
}

#endif