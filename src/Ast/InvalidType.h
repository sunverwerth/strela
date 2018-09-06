// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_InvalidType_h
#define Strela_Ast_InvalidType_h

#include "TypeDecl.h"

namespace Strela {
    class InvalidType: public TypeDecl {
    public:
        InvalidType() { _name = "$invalid"; }
        STRELA_GET_TYPE(Strela::InvalidType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        static InvalidType instance;
    };
}

#endif