#include "UnionType.h"
#include "../exceptions.h"

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
    
    int UnionType::getTypeTag(TypeDecl* t) {
        int tag = 0;
        for (auto&& type: containedTypes) {
            if (type == t) return tag;
            ++tag;
        }
    }

    UnionType::UnionType(const std::set<TypeDecl*>& containedTypes): containedTypes(containedTypes), TypeDecl(getUnionName(containedTypes)) {        
    }
    
    TypeDecl* UnionType::getComplementaryType(TypeDecl* t) {
        if (containedTypes.size() != 2) throw Exception("Calling UnionType::getComplementaryType() on union without exactly 2 contained types.");
        for (auto& ct: containedTypes) {
            if (ct != t) return ct;
        }
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