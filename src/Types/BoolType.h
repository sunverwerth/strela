#ifndef Strela_TypesbooleanType_h
#define Strela_TypesbooleanType_h

#include "Type.h"

namespace Strela {
    class BoolType: public Type {
    public:
        BoolType(): Type("bool") {}
        STRELA_GET_TYPE(Strela::BoolType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;
    };
}
#endif