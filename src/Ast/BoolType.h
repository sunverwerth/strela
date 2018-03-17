#ifndef Strela_Ast_BoolType_h
#define Strela_Ast_BoolType_h

#include "TypeDecl.h"

namespace Strela {
    class BoolType: public TypeDecl {
    public:
        BoolType(const Token& startToken): TypeDecl(startToken, "bool") {}
        STRELA_GET_TYPE(Strela::BoolType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        static BoolType instance;
    };
}

#endif