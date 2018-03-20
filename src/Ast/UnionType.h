#ifndef Strela_Ast_UnionType_h
#define Strela_Ast_UnionType_h

#include "TypeDecl.h"

#include <set>

namespace Strela {
    class UnionType: public TypeDecl {
    public:
        UnionType(const std::set<TypeDecl*>& containedTypes);
        STRELA_GET_TYPE(Strela::UnionType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        static UnionType* get(TypeDecl* left, TypeDecl* right);

    public:
        std::set<TypeDecl*> containedTypes;

        static std::map<std::string, UnionType*> unionTypes;
    };
}

#endif