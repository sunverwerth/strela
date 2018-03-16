#include "EnumType.h"
#include "../Ast/EnumDecl.h"

namespace Strela {
    EnumType::EnumType(EnumDecl* enumDecl): Type(enumDecl->name), enumDecl(enumDecl) {}

    bool EnumType::isAssignableFrom(const Type* other) const {
        return other == this;
    }
}