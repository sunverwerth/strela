// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "InterfaceDecl.h"
#include "InterfaceMethodDecl.h"

namespace Strela {
    Node* InterfaceDecl::getMember(const std::string& name) {
        for (auto&& method: methods) {
            if (method->name == name) {
                return method;
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