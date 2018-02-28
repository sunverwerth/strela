#ifndef Strela_Opcode_h
#define Strela_Opcode_h

#include "VMValue.h"

namespace Strela {
    #define OPCODES(X) \
        X(ReturnVoid, 1, u8) \
        X(Return, 1, u8) \
        X(Const, 1, u8) \
        X(U8, 1, u8) \
        X(I8, 1, i8) \
        X(U16, 2, u16) \
        X(I16, 2, i16) \
        X(U32, 4, u32) \
        X(I32, 4, i32) \
        X(U64, 8, u64) \
        X(I64, 8, i64) \
        X(F32, 4, f32) \
        X(F64, 8, f64) \
        X(Var, 1, u8) \
        X(StoreVar, 1, u8) \
        X(Param, 1, u8) \
        X(StoreParam, 1, u8) \
        X(Field, 1, u8) \
        X(StoreField, 1, u8) \
        X(Element, 0, null) \
        X(StoreElement, 0, null) \
        X(GrowStack, 1, u8) \
        X(Call, 0, null) \
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
        X(New, 1, u8) \
        X(Null, 0, u8) \
        X(Repeat, 0, u8) \
        X(Pop, 0, u8) \
        X(Print, 0, u8) \
        X(AndL, 0, u8) \
        X(OrL, 0, u8) \
    
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