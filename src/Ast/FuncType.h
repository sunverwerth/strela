// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_FuncType_h
#define Strela_Ast_FuncType_h

#include "TypeDecl.h"

#include <vector>

namespace Strela {
    class FuncType: public TypeDecl {
    public:
        STRELA_GET_TYPE(Strela::FuncType, Strela::TypeDecl);

        static FuncType* get(TypeDecl* returnType, const std::vector<TypeDecl*>& paramTypes);

    public:
        std::vector<TypeDecl*> paramTypes;
        TypeDecl* returnType = nullptr;

        static std::vector<FuncType*> funcTypes;
    };
}

#endif