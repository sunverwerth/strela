#include "InterfaceMethodDecl.h"
#include "Param.h"
#include "Expr.h"

#include <sstream>

namespace Strela {
    std::string InterfaceMethodDecl::getDescription() {
        std::stringstream sstr;
        sstr << "function " << name << "(";
        for (auto& param: params) {
            if (param != params.front()) {
                sstr << ", ";
            }
            sstr << param->name << ": " << param->declType->getFullName();
        }
        sstr << "): " << returnTypeExpr->typeValue->getFullName();
        
        return sstr.str();
    }
}