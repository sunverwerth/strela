#ifndef Strela_Types_IntType_h
#define Strela_Types_IntType_h

#include "Type.h"

namespace Strela {
    class IntType: public Type {
    public:
        IntType(bool isSigned, int bytes): Type(std::string(isSigned ? "i" : "u") + std::to_string(bytes*8)), isSigned(isSigned), bytes(bytes) {}
        STRELA_GET_TYPE(Strela::IntType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;
        bool isScalar() const override { return true; }
        Type* getSignedType();

    public:
        bool isSigned;
        int bytes;
    };
}
#endif