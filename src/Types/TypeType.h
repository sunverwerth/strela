#ifndef Strela_Types_TypeType_h
#define Strela_Types_TypeType_h

#include "Type.h"

namespace Strela {
    class TypeType: public Type {
    public:
        TypeType(): Type("$Type") {}
        STRELA_GET_TYPE(Strela::TypeType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;
    };
}
#endif