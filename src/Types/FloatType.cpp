#include "FloatType.h"

namespace Strela {
    bool FloatType::isAssignableFrom(const Type* other) const {
        if (auto ft = other->as<FloatType>()) {
            if (bytes >= ft->bytes) {
                return true;
            }
        }
        return false;
    }
}