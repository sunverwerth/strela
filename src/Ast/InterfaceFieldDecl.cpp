#include "InterfaceFieldDecl.h"
#include "Expr.h"

#include <sstream>

namespace Strela {
    std::string InterfaceFieldDecl::getDescription() {
        std::stringstream sstr;
        sstr << "var " << name << ": " << typeExpr->typeValue->getFullName();
        return sstr.str();
    }
}