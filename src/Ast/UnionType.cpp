// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "UnionType.h"
#include "../exceptions.h"
#include "TypeAliasDecl.h"
#include "Expr.h"

namespace Strela {
    std::set<TypeDecl*> mergeTypes(TypeDecl* left, TypeDecl* right) {
        std::set<TypeDecl*> set;
        if (auto unionLeft = left->as<UnionType>()) {
            set.insert(unionLeft->containedTypes.begin(), unionLeft->containedTypes.end());
        }
        else {
            set.insert(left);
        }
        if (auto unionRight = right->as<UnionType>()) {
            set.insert(unionRight->containedTypes.begin(), unionRight->containedTypes.end());
        }
        else {
            set.insert(right);
        }
        return set;
    }

    std::string getUnionName(const std::set<TypeDecl*>& types) {
        std::string name;
        for (auto&& type: types) {
            if (&type != &*types.begin()) {
                name += "|";
            }
            name += type->getFullName();
        }
        return name;
    }
    
    int UnionType::getTypeTag(TypeDecl* t) {
        if (auto alias = t->as<TypeAliasDecl>()) {
            t = alias->typeExpr->typeValue;
        }

        int tag = 0;
        for (auto&& type: containedTypes) {
            if (type == t) return tag;
            if (auto alias = type->as<TypeAliasDecl>()) {
                if (alias->typeExpr->typeValue == t) {
                    return tag;
                }
            }
            ++tag;
        }
        return -1; // TODO: error handling
    }

    bool UnionType::containsType(TypeDecl* t) {
        for (auto& ct: containedTypes) {
            if (ct == t) {
                return true;
            }
            if (auto alias = ct->as<TypeAliasDecl>()) {
                if (alias->typeExpr->typeValue == t) {
                    return true;
                }
            }
        }
        return false;
    }

    TypeDecl* UnionType::getComplementaryType(TypeDecl* t) {
        if (containedTypes.size() != 2) throw Exception("Calling UnionType::getComplementaryType() on union without exactly 2 contained types.");
        for (auto& ct: containedTypes) {
            if (ct != t) return ct;
        }
        return nullptr; // TODO: error handling
    }

    UnionType* UnionType::get(TypeDecl* left, TypeDecl* right) {
        auto set = mergeTypes(left, right);
        auto name = getUnionName(set);
        auto it = unionTypes.find(name);
        if (it != unionTypes.end()) {
            return it->second;
        }

        auto ut = new UnionType();
        ut->containedTypes = set;
        ut->_name = getUnionName(set);
        unionTypes.insert(std::make_pair(ut->_name, ut));
        return ut;
    }

    std::map<std::string, UnionType*> UnionType::unionTypes;
}