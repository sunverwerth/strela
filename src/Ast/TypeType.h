// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_TypeType_h
#define Strela_Ast_TypeType_h

#include "TypeDecl.h"

namespace Strela {
    class TypeType: public TypeDecl {
    public:
        TypeType() { _name = "Type"; }
        STRELA_GET_TYPE(Strela::TypeType, Strela::TypeDecl);

    public:
        static TypeType instance;
    };
}

#endif