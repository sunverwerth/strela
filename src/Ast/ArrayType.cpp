// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "ArrayType.h"
#include "FieldDecl.h"
#include "IdExpr.h"
#include "IntType.h"

namespace Strela {
    ArrayType* ArrayType::get(TypeDecl* base) {
        auto it = arrayTypes.find(base);
        if (it != arrayTypes.end()) return it->second;
        auto atype = new ArrayType();
        atype->_name = base->getFullName() + "[]";
        atype->baseType = base;
        atype->lengthField = new FieldDecl();
        atype->lengthField->parent = atype;
        atype->lengthField->name = "length";
        atype->lengthField->typeExpr = new IdExpr();
        static_cast<IdExpr*>(atype->lengthField->typeExpr)->name = "u64";
        atype->lengthField->declType = atype->lengthField->typeExpr->type = &IntType::u64;
        atype->lengthField->index = 0;
        arrayTypes.insert(std::make_pair(base, atype));
        return atype;
    }

    Node* ArrayType::getMember(const std::string& name) {
        return name == "length" ? lengthField : nullptr;
    }
}