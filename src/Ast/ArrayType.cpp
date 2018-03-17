#include "ArrayType.h"

namespace Strela {
    ArrayType* ArrayType::get(TypeDecl* base) {
        auto it = arrayTypes.find(base);
        if (it != arrayTypes.end()) return it->second;
        auto atype = new ArrayType(Token(TokenType::Identifier, "", 0, 0), "Array<" + base->name + ">", base);
        arrayTypes.insert(std::make_pair(base, atype));
        return atype;
    }
}