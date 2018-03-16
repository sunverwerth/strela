#ifndef Strela_Ast_AstExpr_h
#define Strela_Ast_AstExpr_h

#include "Node.h"
#include "../Types/types.h"
#include "../IExprVisitor.h"

#define STRELA_IMPL_EXPR_VISITOR void accept(Strela::IExprVisitor& v) override { v.visit(*this); }

namespace Strela {
    class Type;
    class FuncDecl;

    class Expr: public Node {
    public:
        Expr(const Token& startToken): Node(startToken), type(Types::invalid) {}
        STRELA_GET_TYPE(Strela::Expr, Strela::Node);
        virtual void accept(IExprVisitor&) = 0;

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
        
        Param* referencedParam = nullptr;
        FieldDecl* referencedField = nullptr;
        VarDecl* referencedVar = nullptr;
        FuncDecl* referencedFunction = nullptr;
        ModDecl* referencedModule = nullptr;
        EnumDecl* referencedEnum = nullptr;
        EnumElement* referencedEnumElement = nullptr;
        Expr* nodeParent = nullptr;

        std::vector<FuncDecl*> candidates;
    };
}

#endif