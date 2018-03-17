#ifndef Strela_Ast_IntType_h
#define Strela_Ast_IntType_h

#include "TypeDecl.h"

namespace Strela {
    class IntType: public TypeDecl {
    public:
        IntType(const Token& startToken, const std::string& name, bool isSigned, int bytes): TypeDecl(startToken, name), isSigned(isSigned), bytes(bytes) {}
        STRELA_GET_TYPE(Strela::IntType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        bool isSigned;
        int bytes;
        
        static IntType instance;
    };
}

#endif