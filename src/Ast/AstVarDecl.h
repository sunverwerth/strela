#ifndef Strela_Ast_AstVarDecl_h
#define Strela_Ast_AstVarDecl_h

#include "AstStmt.h"
#include "../Types/types.h"

#include <string>

namespace Strela {
    class AstExpr;
    class AstTypeExpr;
    class Type;
    
    class AstVarDecl: public AstStmt {
    public:
        AstVarDecl(const Token& startToken, const std::string& name, AstTypeExpr* typeExpr, AstExpr* initializer): AstStmt(startToken), name(name), typeExpr(typeExpr), initializer(initializer), declType(Types::invalid) {}
        STRELA_GET_TYPE(Strela::AstVarDecl, Strela::AstStmt);
        STRELA_IMPL_VISITOR;

    public:
        std::string name;
        AstTypeExpr* typeExpr;
        AstExpr* initializer;

        int index = 0;
        Type* declType;
    };
}

#endif