#ifndef Strela_Types_Value_h
#define Strela_Types_Value_h

#include <cstdint>

namespace Strela {
    class Type;
    class AstFuncDecl;
    class AstParam;
    class AstExpr;
    class AstNode;

    struct Value {
        Value();

        Type* type = nullptr;
        bool isConst = false;
        bool isMember = false;
        union {
            Type* type;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            float f32;
            double f64;
            bool boolean;
            const char* string;
            AstNode* node;
        } constVal;
        AstParam* param = nullptr;
        AstExpr* parent = nullptr;
    };
}

#endif