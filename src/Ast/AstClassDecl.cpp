#include "AstClassDecl.h"
#include "AstFuncDecl.h"
#include "AstFieldDecl.h"

namespace Strela {
    AstNode* AstClassDecl::getMember(const std::string& name) {
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

    std::vector<AstFuncDecl*> AstClassDecl::getMethods(const std::string& name) {
        std::vector<AstFuncDecl*> met;
        for (auto&& method: methods) {
            if (method->name == name) {
                met.push_back(method);
            }
        }
        return met;
    }

}