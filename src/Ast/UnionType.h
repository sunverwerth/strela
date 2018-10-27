// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_UnionType_h
#define Strela_Ast_UnionType_h

#include "TypeDecl.h"

#include <set>
#include <vector>

namespace Strela {
    class UnionType: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::UnionType, Strela::TypeDecl);

        static UnionType* get(TypeDecl* left, TypeDecl* right);
        TypeDecl* getComplementaryType(TypeDecl* t);
        int getTypeTag(TypeDecl* t);
        bool containsType(TypeDecl* t);

    public:
        std::set<TypeDecl*> containedTypes;

        static std::vector<UnionType*> unionTypes;
    };
}

#endif