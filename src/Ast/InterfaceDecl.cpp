// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "InterfaceDecl.h"
#include "InterfaceMethodDecl.h"
#include "InterfaceFieldDecl.h"

namespace Strela {
    Node* InterfaceDecl::getMember(const std::string& name) {
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

    std::vector<Node*> InterfaceDecl::getMethods(const std::string& name) {
        std::vector<Node*> result;
        for (auto&& method: methods) {
            if (method->name == name) {
                result.push_back(method);
            }
        }
        return result;
    }
}