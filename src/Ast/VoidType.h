// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_VoidType_h
#define Strela_Ast_VoidType_h

#include "TypeDecl.h"

namespace Strela {
    class VoidType: public TypeDecl {
    public:
        VoidType() { _name = "void"; }
        STRELA_GET_TYPE(Strela::VoidType, Strela::TypeDecl);

    public:
        static VoidType instance;
    };
}

#endif