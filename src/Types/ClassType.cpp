#include "ClassType.h"

namespace Strela {
    bool ClassType::isAssignableFrom(const Type* other) const {
        return other == this;
    }
}