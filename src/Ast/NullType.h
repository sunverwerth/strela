// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_NullType_h
#define Strela_Ast_NullType_h

#include "TypeDecl.h"

namespace Strela {
    class NullType: public TypeDecl {
    public:
        NullType() { _name = "null"; }
        STRELA_GET_TYPE(Strela::NullType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        static NullType instance;
    };
}

#endif