#include "AstEnumDecl.h"
#include "AstEnumElement.h"

namespace Strela {
    AstEnumElement* AstEnumDecl::getMember(const std::string& name) {
        for (auto&& element: elements) {
            if (element->name == name) {
                return element;
            }
        }
        return nullptr;
    }
}