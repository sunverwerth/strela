#ifndef Strela_Ast_AstExpr_h
#define Strela_Ast_AstExpr_h

#include "AstNode.h"
#include "../Types/types.h"

namespace Strela {
    class Type;
    class AstFuncDecl;

    class AstExpr: public AstNode {
    public:
        AstExpr(const Token& startToken): AstNode(startToken), type(Types::invalid) {}
        STRELA_GET_TYPE(Strela::AstExpr, Strela::AstNode);

    public:
        Type* type;
        bool isStatic = false;
        union {
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
            Type* type;
        } staticValue;
        
        AstParam* referencedParam = nullptr;
        AstFieldDecl* referencedField = nullptr;
        AstVarDecl* referencedVar = nullptr;
        AstFuncDecl* referencedFunction = nullptr;
        AstModDecl* referencedModule = nullptr;
        AstExpr* nodeParent = nullptr;

        std::vector<AstFuncDecl*> candidates;
    };
}

#endif