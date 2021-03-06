// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_PointerType_h
#define Strela_Ast_PointerType_h

#include "TypeDecl.h"

namespace Strela {
    class PointerType: public TypeDecl {
    public:
        PointerType() { _name = "Ptr"; }
        STRELA_GET_TYPE(Strela::PointerType, Strela::TypeDecl);

    public:        
        static PointerType instance;
    };
}

#endif