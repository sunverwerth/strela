#ifndef Strela_Ast_AstClassDecl_h
#define Strela_Ast_AstClassDecl_h

#include "TypeDecl.h"

#include <string>
#include <vector>

namespace Strela {
    class FuncDecl;
    class FieldDecl;
    
    class ClassDecl: public TypeDecl {
    public:
        ClassDecl(const Token& startToken, const std::string& name, const std::vector<FuncDecl*>& methods, const std::vector<FieldDecl*>& fields): TypeDecl(startToken, name), methods(methods), fields(fields) {}
        STRELA_GET_TYPE(Strela::ClassDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        std::vector<FuncDecl*> getMethods(const std::string& name);

    public:
        bool isExported = false;
        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;
        static ClassDecl String;
    };
}

#endif