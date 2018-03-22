#ifndef Strela_Ast_AstGenericParam_h
#define Strela_Ast_AstGenericParam_h

#include "TypeDecl.h"

namespace Strela {
    class GenericParam: public TypeDecl {
    public:
        GenericParam(const std::string& name): TypeDecl(name) {}
        STRELA_GET_TYPE(Strela::GenericParam, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;
    };
}

#endif