// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstClassDecl_h
#define Strela_Ast_AstClassDecl_h

#include "TypeDecl.h"

#include <string>
#include <vector>

namespace Strela {
    class FuncDecl;
    class FieldDecl;
    class GenericParam;
    
    class ClassDecl: public TypeDecl {
    public:
        ClassDecl() = default;
        ClassDecl(const std::string& name) { this->_name = name; }
        STRELA_GET_TYPE(Strela::ClassDecl, Strela::TypeDecl);

        Node* getMember(const std::string& name) override;
        std::vector<Node*> getMethods(const std::string& name) override;

        ClassDecl* getReifiedClass(const std::vector<TypeDecl*>& typeArgs);

    public:
        bool isResolved = false;
        bool isExported = false;
        std::vector<GenericParam*> genericParams;
        std::vector<TypeDecl*> genericArguments;
        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;
        static ClassDecl* String;
        std::vector<ClassDecl*> reifiedClasses;
        ClassDecl* genericBase = nullptr;
    };
}

#endif