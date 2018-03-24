// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_ArrayType_h
#define Strela_Ast_ArrayType_h

#include "TypeDecl.h"

namespace Strela {
    class FieldDecl;
    class ArrayType: public TypeDecl {
    public:
        ArrayType(const std::string& name, TypeDecl* baseType): TypeDecl(name), baseType(baseType) {}
        STRELA_GET_TYPE(Strela::ArrayType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name) override;

        static ArrayType* get(TypeDecl* base);

    public:
        TypeDecl* baseType;
        FieldDecl* lengthField;
        static std::map<TypeDecl*, ArrayType*> arrayTypes;
    };
}

#endif