#ifndef Strela_Ast_AstParam_h
#define Strela_Ast_AstParam_h

#include "AstStmt.h"
#include "../Types/types.h"

#include <string>

namespace Strela {
    class AstTypeExpr;
    class Type;
    
    class AstParam: public AstStmt {
    public:
        AstParam(const Token& startToken, const std::string& name, AstTypeExpr* typeExpr): AstStmt(startToken), name(name), typeExpr(typeExpr), declType(Types::invalid) {}
        STRELA_GET_TYPE(Strela::AstParam, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        int index = 0;
        std::string name;
        AstTypeExpr* typeExpr;
        Type* declType;
    };
}

#endif