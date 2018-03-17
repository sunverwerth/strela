#ifndef Strela_Ast_NullType_h
#define Strela_Ast_NullType_h

#include "TypeDecl.h"

namespace Strela {
    class NullType: public TypeDecl {
    public:
        NullType(const Token& startToken): TypeDecl(startToken, "null") {}
        STRELA_GET_TYPE(Strela::NullType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        static NullType instance;
    };
}

#endif