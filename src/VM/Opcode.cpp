// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "Opcode.h"

namespace Strela {

    #define AS_INFO(X, A, T) { #X, A, VMValue::Type::T },
    OpcodeInfo opcodeInfo[] {
        OPCODES(AS_INFO)
    };
    #undef AS_INFO
}