// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_IntType_h
#define Strela_Ast_IntType_h

#include "TypeDecl.h"

namespace Strela {
    class IntType: public TypeDecl {
    public:
        IntType(const std::string& name, bool isSigned, int bytes): isSigned(isSigned), bytes(bytes) { this->_name = name; }
        STRELA_GET_TYPE(Strela::IntType, Strela::TypeDecl);

    public:
        bool isSigned;
        int bytes;
        
        static IntType u8;
        static IntType u16;
        static IntType u32;
        static IntType u64;
        static IntType i8;
        static IntType i16;
        static IntType i32;
        static IntType i64;
    };
}

#endif