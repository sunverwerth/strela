#ifndef Strela_Ast_VoidType_h
#define Strela_Ast_VoidType_h

#include "TypeDecl.h"

namespace Strela {
    class VoidType: public TypeDecl {
    public:
        VoidType(): TypeDecl("void") {}
        STRELA_GET_TYPE(Strela::VoidType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

    public:
        static VoidType instance;
    };
}

#endif