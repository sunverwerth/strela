// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_FloatType_h
#define Strela_Ast_FloatType_h

#include "TypeDecl.h"

namespace Strela {
    class FloatType: public TypeDecl {
    public:
        FloatType(const std::string& name, int bytes): bytes(bytes) { this->_name = name; }
        STRELA_GET_TYPE(Strela::FloatType, Strela::TypeDecl);

    public:
        int bytes;
        
        static FloatType f64;
        static FloatType f32;
    };
}

#endif