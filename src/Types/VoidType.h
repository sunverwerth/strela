#ifndef Strela_Types_VoidType_h
#define Strela_Types_VoidType_h

#include "Type.h"

namespace Strela {
    class VoidType: public Type {
    public:
        VoidType(): Type("void") {}
        STRELA_GET_TYPE(Strela::VoidType, Strela::Type);
    };
}
#endif