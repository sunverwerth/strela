#ifndef Strela_Types_ArrayType_h
#define Strela_Types_ArrayType_h

#include "Type.h"

#include <map>

namespace Strela {
    class ArrayType: public Type {
    public:
        ArrayType(Type* base): Type(base->name + "[]") {}
        STRELA_GET_TYPE(Strela::ArrayType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override { return other == this; };
        static ArrayType* get(Type* base);

    public:
        static std::map<Type*, ArrayType*> arrayTypes;
    };
}
#endif