#include "IntType.h"
#include "types.h"

namespace Strela {
    bool IntType::isAssignableFrom(const Type* other) const {
        if (auto otherType = other->as<IntType>()) {
            if (isSigned == otherType->isSigned && bytes >= otherType->bytes) {
                return true;
            }
            if (isSigned && bytes > otherType->bytes) {
                return true;
            }
        }
        return false;
    }

    Type* IntType::getSignedType() {
        if (isSigned) return this;
        if (this == Types::u8) return Types::i8;
        if (this == Types::u16) return Types::i16;
        if (this == Types::u32) return Types::i32;
        if (this == Types::u64) return Types::i64;
        return Types::invalid;
    }
}