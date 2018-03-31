// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "ArrayType.h"
#include "FieldDecl.h"
#include "IdTypeExpr.h"
#include "IntType.h"

namespace Strela {
    ArrayType* ArrayType::get(TypeDecl* base) {
        auto it = arrayTypes.find(base);
        if (it != arrayTypes.end()) return it->second;
        auto atype = new ArrayType(base->name + "[]", base);
        atype->lengthField = new FieldDecl("length", new IdTypeExpr("u64"));
        atype->lengthField->type = atype->lengthField->typeExpr->type = &IntType::u64;
        atype->lengthField->index = 0;
        arrayTypes.insert(std::make_pair(base, atype));
        return atype;
    }

    Node* ArrayType::getMember(const std::string& name) {
        return name == "length" ? lengthField : nullptr;
    }
}