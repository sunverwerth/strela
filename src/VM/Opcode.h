// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_Opcode_h
#define Strela_VM_Opcode_h

#include "VMValue.h"

namespace Strela {
    #define OPCODES(X) \
        X(Trap, 0, null) \
        X(ReturnVoid, 0, null) \
        X(Return, 0, null) \
        X(Const, 2, integer) \
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
        X(Ptr8, 1, integer) \
        X(Ptr16, 1, integer) \
        X(Ptr32, 1, integer) \
        X(Ptr64, 1, integer) \
        X(Ptr64Var, 2, integer) \
        X(ObjPtr64, 1, integer) \
        X(ObjPtr64Var, 2, integer) \
        X(PtrInd8, 1, integer) \
        X(PtrInd16, 1, integer) \
        X(PtrInd32, 1, integer) \
        X(PtrInd64, 1, integer) \
        X(ObjPtrInd64, 1, integer) \
        X(StorePtr8, 1, integer) \
        X(StorePtr16, 1, integer) \
        X(StorePtr32, 1, integer) \
        X(StorePtr64, 1, integer) \
        X(StorePtr64Var, 2, integer) \
        X(StorePtrInd8, 1, integer) \
        X(StorePtrInd16, 1, integer) \
        X(StorePtrInd32, 1, integer) \
        X(StorePtrInd64, 1, integer) \
        X(Call, 1, integer) \
        X(CallImm, 5, integer) \
        X(NativeCall, 0, null) \
		X(BuiltinCall, 8, integer) \
        X(Jmp, 0, null) \
        X(JmpIf, 0, null) \
        X(JmpIfNot, 0, null) \
        X(CmpEQ, 0, null) \
        X(CmpNE, 0, null) \
        X(CmpLTI, 0, null) \
        X(CmpLTF32, 0, null) \
        X(CmpLTF64, 0, null) \
        X(CmpGTI, 0, null) \
        X(CmpGTF32, 0, null) \
        X(CmpGTF64, 0, null) \
        X(CmpLTE, 0, null) \
        X(CmpGTE, 0, null) \
        X(Not, 0, null) \
        X(AddI, 0, null) \
        X(AddF32, 0, null) \
        X(AddF64, 0, null) \
        X(SubI, 0, null) \
        X(SubF32, 0, null) \
        X(SubF64, 0, null) \
        X(MulI, 0, null) \
        X(MulF32, 0, null) \
        X(MulF64, 0, null) \
        X(DivI, 0, null) \
        X(DivF32, 0, null) \
        X(DivF64, 0, null) \
        X(ModI, 0, null) \
        X(New, 2, integer) \
        X(Array, 0, null) \
        X(Null, 0, null) \
        X(Repeat, 0, null) \
        X(Pop, 0, null) \
        X(PrintI, 0, null) \
        X(PrintF32, 0, null) \
        X(PrintF64, 0, null) \
        X(PrintS, 0, null) \
        X(PrintN, 0, null) \
        X(PrintO, 0, null) \
        X(PrintB, 0, null) \
        X(AndL, 0, null) \
        X(OrL, 0, null) \
        X(Swap, 0, null) \
        X(I64tF32, 0, null) \
        X(I64tF64, 0, null) \
        X(F32tI64, 0, null) \
        X(F64tI64, 0, null) \
        X(F64tF32, 0, null) \
        X(F32tF64, 0, null) \
        X(Peek, 1, integer) \
        X(Mov8, 0, null) \
        X(Mov16, 0, null) \
        X(Mov32, 0, null) \
        X(Mov64, 0, null) \
        X(CmpType, 8, integer) \
    
    #define AS_ENUM(X, A, T) X,
    enum class Opcode: unsigned char {
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

    struct AddrMode {
        int dest: 2;
        int src1: 3;
        int src2: 3;
    };

    extern OpcodeInfo opcodeInfo[];
}

#endif