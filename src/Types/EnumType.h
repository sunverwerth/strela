#ifndef Strela_Types_EnumType_h
#define Strela_Types_EnumType_h

#include "Type.h"

namespace Strela {
    class AstEnumDecl;

    class EnumType: public Type {
    public:
        EnumType(AstEnumDecl* enumDecl);
        STRELA_GET_TYPE(Strela::EnumType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;

    public:
        AstEnumDecl* enumDecl;
    };
}
#endif