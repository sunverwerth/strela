#ifndef Strela_Types_EnumType_h
#define Strela_Types_EnumType_h

#include "Type.h"

namespace Strela {
    class EnumDecl;

    class EnumType: public Type {
    public:
        EnumType(EnumDecl* enumDecl);
        STRELA_GET_TYPE(Strela::EnumType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;

    public:
        EnumDecl* enumDecl;
    };
}
#endif