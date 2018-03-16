#ifndef Strela_Ast_AstVarDecl_h
#define Strela_Ast_AstVarDecl_h

#include "Stmt.h"
#include "../Types/types.h"

#include <string>

namespace Strela {
    class Expr;
    class TypeExpr;
    class Type;
    
    class VarDecl: public Stmt {
    public:
        VarDecl(const Token& startToken, const std::string& name, TypeExpr* typeExpr, Expr* initializer): Stmt(startToken), name(name), typeExpr(typeExpr), initializer(initializer), declType(Types::invalid) {}
        STRELA_GET_TYPE(Strela::VarDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        TypeExpr* typeExpr;
        Expr* initializer;

        int index = 0;
        Type* declType;
    };
}

#endif