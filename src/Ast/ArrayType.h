// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_ArrayType_h
#define Strela_Ast_ArrayType_h

#include "TypeDecl.h"

namespace Strela {
    class FieldDecl;
    
    class ArrayType: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::ArrayType, Strela::TypeDecl);

        Node* getMember(const std::string& name) override;

        static ArrayType* get(TypeDecl* base);

    public:
        TypeDecl* baseType = nullptr;
        FieldDecl* lengthField = nullptr;
        static std::map<TypeDecl*, ArrayType*> arrayTypes;
    };
}

#endif