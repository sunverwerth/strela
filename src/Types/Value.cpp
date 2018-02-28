#include "Value.h"
#include "types.h"

namespace Strela {
    Value::Value() {
        type = Types::invalid;
        constVal.u64 = 0;
    }
}