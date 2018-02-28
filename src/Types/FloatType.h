#ifndef Strela_Types_FloatType_h
#define Strela_Types_FloatType_h

#include "Type.h"

namespace Strela {
    class FloatType: public Type {
    public:
        FloatType(int bytes): Type("f" + std::to_string(bytes*8)), bytes(bytes) {}
        STRELA_GET_TYPE(Strela::FloatType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;
        bool isScalar() const override { return true; }
        
    public:
        int bytes;
    };
}
#endif