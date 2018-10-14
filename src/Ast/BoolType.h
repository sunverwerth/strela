// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_BoolType_h
#define Strela_Ast_BoolType_h

#include "TypeDecl.h"

namespace Strela {
    class BoolType: public TypeDecl {
    public:
        BoolType() { _name = "bool"; }
        STRELA_GET_TYPE(Strela::BoolType, Strela::TypeDecl);

    public:
        static BoolType instance;
    };
}

#endif