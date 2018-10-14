// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstModDecl_h
#define Strela_Ast_AstModDecl_h

#include "TypeDecl.h"

#include <vector>
#include <string>

namespace Strela {
    class FuncDecl;
    class ClassDecl;
    class InterfaceDecl;
    class ModuleType;
    class EnumDecl;
    class ImportStmt;
    class TypeAliasDecl;

    class ModDecl: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::ModDecl, Strela::TypeDecl);

        Node* getMember(const std::string& name) override;
        ClassDecl* getClass(const std::string& name);
        TypeAliasDecl* getAlias(const std::string& name);
        EnumDecl* getEnum(const std::string& name);
        std::vector<FuncDecl*> getFunctions(const std::string& name);
        void addFunction(FuncDecl* func);
        void addClass(ClassDecl* cls);

    public:
        std::vector<ImportStmt*> imports;
        std::vector<FuncDecl*> functions;
        std::vector<ClassDecl*> classes;
        std::vector<InterfaceDecl*> interfaces;
        std::vector<EnumDecl*> enums;
        std::vector<TypeAliasDecl*> typeAliases;
        std::string filename;
    };
}

#endif