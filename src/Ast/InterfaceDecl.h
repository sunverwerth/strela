// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstInterfaceDecl_h
#define Strela_Ast_AstInterfaceDecl_h

#include "TypeDecl.h"

#include <string>
#include <vector>

namespace Strela {
    class InterfaceDecl;
    class InterfaceMethodDecl;
    class InterfaceFieldDecl;
    class ClassDecl;
    class FuncDecl;
    class FieldDecl;

    class Implementation {
    public:
        InterfaceDecl* interface;
        ClassDecl* _class;
        std::vector<FuncDecl*> classMethods;
        std::vector<FieldDecl*> classFields;
    };
    
    class InterfaceDecl: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::InterfaceDecl, Strela::TypeDecl);

        Node* getMember(const std::string& name) override;
        std::vector<Node*> getMethods(const std::string& name) override;

    public:
        bool isExported = false;
        std::vector<InterfaceMethodDecl*> methods;
        std::vector<InterfaceFieldDecl*> fields;

        std::map<ClassDecl*, Implementation*> implementations;
    };
}

#endif