#ifndef Strela_Ast_AstFieldDecl_h
#define Strela_Ast_AstFieldDecl_h

#include "AstStmt.h"

#include <string>

namespace Strela {
    class AstTypeExpr;
    class Type;
    
    class AstFieldDecl: public AstStmt {
    public:
        AstFieldDecl(const Token& startToken, const std::string& name, AstTypeExpr* typeExpr): AstStmt(startToken), name(name), typeExpr(typeExpr) {}
        STRELA_GET_TYPE(Strela::AstFieldDecl, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        AstTypeExpr* typeExpr;

        int index = 0;
        Type* declType = nullptr;
    };
}

#endif