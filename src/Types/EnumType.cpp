#include "EnumType.h"
#include "../Ast/AstEnumDecl.h"

namespace Strela {
    EnumType::EnumType(AstEnumDecl* enumDecl): Type(enumDecl->name), enumDecl(enumDecl) {}

    bool EnumType::isAssignableFrom(const Type* other) const {
        return other == this;
    }
}