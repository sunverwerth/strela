// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_OverloadedFuncType_h
#define Strela_Ast_OverloadedFuncType_h

#include "TypeDecl.h"

namespace Strela {
    class OverloadedFuncType: public TypeDecl {
    public:
        OverloadedFuncType() { _name = "$overloaded"; }
        STRELA_GET_TYPE(Strela::OverloadedFuncType, Strela::TypeDecl);

    public:
        static OverloadedFuncType instance;
    };
}

#endif