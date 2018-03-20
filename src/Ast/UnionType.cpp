#include "UnionType.h"

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
            name += type->name;
        }
        return name;
    }

    UnionType::UnionType(const std::set<TypeDecl*>& containedTypes): containedTypes(containedTypes), TypeDecl(getUnionName(containedTypes)) {        
    }

    UnionType* UnionType::get(TypeDecl* left, TypeDecl* right) {
        auto set = mergeTypes(left, right);
        auto name = getUnionName(set);
        auto it = unionTypes.find(name);
        if (it != unionTypes.end()) {
            return it->second;
        }

        auto ut = new UnionType(set);
        unionTypes.insert(std::make_pair(ut->name, ut));
        return ut;
    }

    std::map<std::string, UnionType*> UnionType::unionTypes;
}