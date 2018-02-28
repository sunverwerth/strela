#include "TypeType.h"

namespace Strela {
    bool TypeType::isAssignableFrom(const Type* other) const {
        return other == this;
    }
}