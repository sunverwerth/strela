// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

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
            sstr << p->getFullName();
            if (&p != &paramTypes.back()) {
                sstr << ", ";
            }
        }
        sstr << "): " << returnType->getFullName();
        auto ftype = new FuncType();
        ftype->_name = sstr.str();
        ftype->returnType = returnType;
        ftype->paramTypes = paramTypes;
        funcTypes.push_back(ftype);
        return ftype;
    }
}