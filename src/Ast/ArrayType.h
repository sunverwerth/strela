#ifndef Strela_Ast_ArrayType_h
#define Strela_Ast_ArrayType_h

#include "TypeDecl.h"

namespace Strela {
    class ArrayType: public TypeDecl {
    public:
        ArrayType(const std::string& name, TypeDecl* baseType): TypeDecl(name), baseType(baseType) {}
        STRELA_GET_TYPE(Strela::ArrayType, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        static ArrayType* get(TypeDecl* base);

    public:
        TypeDecl* baseType;
        static std::map<TypeDecl*, ArrayType*> arrayTypes;
    };
}

#endif