#ifndef Strela_Ast_AstClassDecl_h
#define Strela_Ast_AstClassDecl_h

#include "AstStmt.h"
#include "../Types/types.h"

#include <string>
#include <vector>

namespace Strela {
    class AstFuncDecl;
    class AstFieldDecl;
    class Type;
    
    class AstClassDecl: public AstStmt {
    public:
        AstClassDecl(const Token& startToken, const std::string& name, const std::vector<AstFuncDecl*>& methods, const std::vector<AstFieldDecl*>& fields): AstStmt(startToken), name(name), methods(methods), fields(fields), declType(new ClassType(name, this)) {}
        STRELA_GET_TYPE(Strela::AstClassDecl, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

        AstNode* getMember(const std::string& name);
        std::vector<AstFuncDecl*> getMethods(const std::string& name);

    public:
        bool isExported = false;
        std::string name;
        std::vector<AstFuncDecl*> methods;
        std::vector<AstFieldDecl*> fields;

        Type* declType;
    };
}

#endif