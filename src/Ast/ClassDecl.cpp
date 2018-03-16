#include "ClassDecl.h"
#include "FuncDecl.h"
#include "FieldDecl.h"

namespace Strela {
    Node* ClassDecl::getMember(const std::string& name) {
        for (auto&& method: methods) {
            if (method->name == name) {
                return method;
            }
        }
        for (auto&& field: fields) {
            if (field->name == name) {
                return field;
            }
        }
        return nullptr;
    }

    std::vector<FuncDecl*> ClassDecl::getMethods(const std::string& name) {
        std::vector<FuncDecl*> met;
        for (auto&& method: methods) {
            if (method->name == name) {
                met.push_back(method);
            }
        }
        return met;
    }

}