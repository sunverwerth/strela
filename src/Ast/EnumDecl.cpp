// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "EnumDecl.h"
#include "EnumElement.h"

namespace Strela {
    Node* EnumDecl::getMember(const std::string& name) {
        for (auto&& element: elements) {
            if (element->name == name) {
                return element;
            }
        }
        return nullptr;
    }
}