#include "ClassDecl.h"
#include "FuncDecl.h"
#include "FieldDecl.h"
#include "../Parser.h"

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

    std::vector<Node*> ClassDecl::getMethods(const std::string& name) {
        std::vector<Node*> met;
        for (auto&& method: methods) {
            if (method->name == name) {
                met.push_back(method);
            }
        }
        return met;
    }

    ClassDecl* ClassDecl::getReifiedClass(const std::vector<TypeDecl*>& typeArgs, bool& isnew) {
        for (auto& rc: reifiedClasses) {
            bool found = true;
            for (int i = 0; i < typeArgs.size(); ++i) {
                if (typeArgs[i] != rc->genericArguments[i]) {
                    found = false;
                    break;
                }
            }
            if (found)  {
                isnew = false;
                return rc;
            }
        }

        // copy class definition
        Parser parser(*source, firstToken);
        auto cls = parser.parseClassDecl();
        // set type arguments
        cls->genericArguments = typeArgs;
        reifiedClasses.push_back(cls);
        isnew = true;
        return cls;
    }
}