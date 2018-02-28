#include "AstModDecl.h"
#include "AstFuncDecl.h"
#include "AstClassDecl.h"

namespace Strela {
    AstNode* AstModDecl::getMember(const std::string& name) {
        for (auto&& func: functions) {
            if (func->name == name) {
                return func;
            }
        }
        for (auto&& cls: classes) {
            if (cls->name == name) {
                return cls;
            }
        }
        return nullptr;
    }

    AstClassDecl* AstModDecl::getClass(const std::string& name) {        
        for (auto&& cls: classes) {
            if (cls->name == name) {
                return cls;
            }
        }
        return nullptr;
    }

    std::vector<AstFuncDecl*> AstModDecl::getFunctions(const std::string& name) {
        std::vector<AstFuncDecl*> funcs;
        for (auto&& function: functions) {
            if (function->name == name) {
                funcs.push_back(function);
            }
        }
        return funcs;
    }

    void AstModDecl::addFunction(AstFuncDecl* func) {
        functions.push_back(func);
    }

    void AstModDecl::addClass(AstClassDecl* cls) {
        classes.push_back(cls);
    }
}