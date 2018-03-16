#ifndef Strela_Ast_AstClassDecl_h
#define Strela_Ast_AstClassDecl_h

#include "Stmt.h"
#include "../Types/types.h"

#include <string>
#include <vector>

namespace Strela {
    class FuncDecl;
    class FieldDecl;
    class Type;
    
    class ClassDecl: public Stmt {
    public:
        ClassDecl(const Token& startToken, const std::string& name, const std::vector<FuncDecl*>& methods, const std::vector<FieldDecl*>& fields): Stmt(startToken), name(name), methods(methods), fields(fields), declType(new ClassType(name, this)) {}
        STRELA_GET_TYPE(Strela::ClassDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        std::vector<FuncDecl*> getMethods(const std::string& name);

    public:
        bool isExported = false;
        std::string name;
        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;

        Type* declType;
    };
}

#endif