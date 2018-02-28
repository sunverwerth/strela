#include "ArrayType.h"

namespace Strela {
    ArrayType* ArrayType::get(Type* base) {
        auto it = arrayTypes.find(base);
        if (it != arrayTypes.end()) return it->second;
        auto at = new ArrayType(base);
        arrayTypes[base] = at;
        return at;
    }

    std::map<Type*, ArrayType*> ArrayType::arrayTypes;
}