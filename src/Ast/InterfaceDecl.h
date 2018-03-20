#ifndef Strela_Ast_AstInterfaceDecl_h
#define Strela_Ast_AstInterfaceDecl_h

#include "TypeDecl.h"

#include <string>
#include <vector>

namespace Strela {
    class InterfaceMethodDecl;

    class Implementation {
    public:
        InterfaceDecl* interface;
        ClassDecl* _class;
        std::vector<FuncDecl*> classMethods;
    };
    
    class InterfaceDecl: public TypeDecl {
    public:
        InterfaceDecl(const std::string& name, const std::vector<InterfaceMethodDecl*>& methods): TypeDecl(name), methods(methods) {}
        STRELA_GET_TYPE(Strela::InterfaceDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        std::vector<InterfaceMethodDecl*> getMethods(const std::string& name);

    public:
        bool isExported = false;
        std::vector<InterfaceMethodDecl*> methods;

        std::map<ClassDecl*, Implementation*> implementations;
    };
}

#endif