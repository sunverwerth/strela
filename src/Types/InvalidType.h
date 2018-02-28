#ifndef Strela_Types_InvalidType_h
#define Strela_Types_InvalidType_h

#include "Type.h"

namespace Strela {
    class InvalidType: public Type {
    public:
        InvalidType(): Type("$Invalid") {}
        STRELA_GET_TYPE(Strela::InvalidType, Strela::Type);
    };
}
#endif