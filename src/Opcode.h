// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Opcode_h
#define Strela_Opcode_h

#include "VMValue.h"

namespace Strela {
    #define OPCODES(X) \
        X(ReturnVoid, 0, null) \
        X(Return, 0, null) \
        X(Const, 1, integer) \
        X(I8, 1, integer) \
        X(I16, 2, integer) \
        X(I32, 4, integer) \
        X(I64, 8, integer) \
        X(U8, 1, integer) \
        X(U16, 2, integer) \
        X(U32, 4, integer) \
        X(U64, 8, integer) \
        X(F32, 4, floating) \
        X(F64, 8, floating) \
        X(Grow, 1, integer) \
        X(Var, 1, integer) \
        X(StoreVar, 1, integer) \
        X(Field, 1, integer) \
        X(FieldInd, 1, integer) \
        X(StoreField, 1, integer) \
        X(StoreFieldInd, 1, integer) \
        X(Call, 1, integer) \
        X(NativeCall, 0, null) \
        X(Jmp, 0, null) \
        X(JmpIf, 0, null) \
        X(JmpIfNot, 0, null) \
        X(CmpEQ, 0, null) \
        X(CmpNE, 0, null) \
        X(CmpLT, 0, null) \
        X(CmpGT, 0, null) \
        X(CmpLTE, 0, null) \
        X(CmpGTE, 0, null) \
        X(Not, 0, null) \
        X(Add, 0, null) \
        X(Sub, 0, null) \
        X(Mul, 0, null) \
        X(Div, 0, null) \
        X(New, 0, null) \
        X(Null, 0, null) \
        X(Repeat, 0, null) \
        X(Pop, 0, null) \
        X(Print, 0, null) \
        X(AndL, 0, null) \
        X(OrL, 0, null) \
        X(Swap, 0, null) \
        X(I64tF64, 0, null) \
        X(F64tI64, 0, null) \
        X(Peek, 1, integer) \
    
    #define AS_ENUM(X, A, T) X,
    enum class Opcode {
        OPCODES(AS_ENUM)
    };
    #undef AS_ENUM

    #define AS_COUNT(C, A, T) + 1
    const int numOpcodes = 0 OPCODES(AS_COUNT);
    #undef AS_COUNT

    struct OpcodeInfo {
        const char* name;
        unsigned char argWidth;
        VMValue::Type argType;
    };

    extern OpcodeInfo opcodeInfo[];
}

#endif