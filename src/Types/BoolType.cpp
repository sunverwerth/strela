#include "BoolType.h"

namespace Strela {
    bool BoolType::isAssignableFrom(const Type* other) const {
        return other == this;
    }
}