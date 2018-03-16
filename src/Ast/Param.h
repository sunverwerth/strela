#ifndef Strela_Ast_AstParam_h
#define Strela_Ast_AstParam_h

#include "Stmt.h"
#include "../Types/types.h"

#include <string>

namespace Strela {
    class TypeExpr;
    class Type;
    
    class Param: public Stmt {
    public:
        Param(const Token& startToken, const std::string& name, TypeExpr* typeExpr): Stmt(startToken), name(name), typeExpr(typeExpr), declType(Types::invalid) {}
        STRELA_GET_TYPE(Strela::Param, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        int index = 0;
        std::string name;
        TypeExpr* typeExpr;
        Type* declType;
    };
}

#endif