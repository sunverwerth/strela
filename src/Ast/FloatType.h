#ifndef Strela_Ast_FloatType_h
#define Strela_Ast_FloatType_h

#include "TypeDecl.h"

namespace Strela {
    class FloatType: public TypeDecl {
    public:
        FloatType(const Token& startToken, const std::string& name, int bytes): TypeDecl(startToken, name), bytes(bytes) {}
        STRELA_GET_TYPE(Strela::FloatType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        int bytes;
        
        static FloatType f32;
        static FloatType f64;
    };
}

#endif