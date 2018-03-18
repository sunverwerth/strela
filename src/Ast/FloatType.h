#ifndef Strela_Ast_FloatType_h
#define Strela_Ast_FloatType_h

#include "TypeDecl.h"

namespace Strela {
    class FloatType: public TypeDecl {
    public:
        FloatType(const std::string& name, int bytes): TypeDecl(name), bytes(bytes) {}
        STRELA_GET_TYPE(Strela::FloatType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        int bytes;
        
        static FloatType instance;
    };
}

#endif