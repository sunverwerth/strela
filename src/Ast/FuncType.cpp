#include "FuncType.h"

#include <sstream>

namespace Strela {
    FuncType* FuncType::get(TypeDecl* returnType, const std::vector<TypeDecl*>& paramTypes) {
        for (auto&& ftype: funcTypes) {
            if (ftype->returnType == returnType && ftype->paramTypes == paramTypes) return ftype;
        }

        std::stringstream sstr;
        sstr << "(";
        for (auto&& p: paramTypes) {
            sstr << p->name;
            if (&p != &paramTypes.back()) {
                sstr << ", ";
            }
        }
        sstr << "): " << returnType->name;
        auto ftype = new FuncType(Token(TokenType::Identifier, "", 0, 0), sstr.str());
        ftype->returnType = returnType;
        ftype->paramTypes = paramTypes;
        funcTypes.push_back(ftype);
        return ftype;
    }
}