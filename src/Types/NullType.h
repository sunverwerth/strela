#ifndef Strela_Types_NullType_h
#define Strela_Types_NullType_h

#include "Type.h"

namespace Strela {
    class NullType: public Type {
    public:
        NullType(): Type("null") {}
        STRELA_GET_TYPE(Strela::NullType, Strela::Type);
    };
}
#endif