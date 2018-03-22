#ifndef Strela_Opcode_h
#define Strela_Opcode_h

#include "VMValue.h"

namespace Strela {
    #define OPCODES(X) \
        X(ReturnVoid, 1, integer) \
        X(Return, 1, integer) \
        X(Const, 1, integer) \
        X(INT, 8, integer) \
        X(FLOAT, 8, floating) \
        X(Var, 1, integer) \
        X(StoreVar, 1, integer) \
        X(Param, 1, integer) \
        X(StoreParam, 1, integer) \
        X(Field, 1, integer) \
        X(FieldInd, 1, integer) \
        X(StoreField, 1, integer) \
        X(Element, 0, null) \
        X(StoreElement, 0, null) \
        X(GrowStack, 1, integer) \
        X(Call, 0, null) \
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
        X(New, 1, integer) \
        X(Null, 0, null) \
        X(Repeat, 0, null) \
        X(Pop, 0, null) \
        X(Print, 0, null) \
        X(AndL, 0, null) \
        X(OrL, 0, null) \
        X(Swap, 0, null) \
        X(I2F, 0, null) \
        X(F2I, 0, null) \
    
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