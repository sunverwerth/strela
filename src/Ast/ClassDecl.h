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
        ClassDecl(const std::string& name, const std::vector<GenericParam*>& genericParams, const std::vector<FuncDecl*>& methods, const std::vector<FieldDecl*>& fields): TypeDecl(name), genericParams(genericParams), methods(methods), fields(fields) {}
        STRELA_GET_TYPE(Strela::ClassDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        std::vector<Node*> getMethods(const std::string& name) override;

        ClassDecl* getReifiedClass(const std::vector<TypeDecl*>& typeArgs, bool& isnew);

    public:
        bool isExported = false;
        std::vector<GenericParam*> genericParams;
        std::vector<TypeDecl*> genericArguments;
        std::vector<FuncDecl*> methods;
        std::vector<FieldDecl*> fields;
        static ClassDecl String;
        std::vector<ClassDecl*> reifiedClasses;
    };
}

#endif