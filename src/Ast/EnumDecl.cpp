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