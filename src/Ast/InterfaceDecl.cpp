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

    std::vector<InterfaceMethodDecl*> InterfaceDecl::getMethods(const std::string& name) {
        std::vector<InterfaceMethodDecl*> result;
        for (auto&& method: methods) {
            if (method->name == name) {
                result.push_back(method);
            }
        }
        return result;
    }
}