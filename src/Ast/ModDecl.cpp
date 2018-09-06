// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "ModDecl.h"
#include "FuncDecl.h"
#include "ClassDecl.h"
#include "EnumDecl.h"

namespace Strela {
    Node* ModDecl::getMember(const std::string& name) {
        for (auto&& func: functions) {
            if (func->name == name) {
                return func;
            }
        }
        for (auto&& cls: classes) {
            if (cls->_name == name) {
                return cls;
            }
        }
        return nullptr;
    }

    ClassDecl* ModDecl::getClass(const std::string& name) {        
        for (auto&& cls: classes) {
            if (cls->_name == name) {
                return cls;
            }
        }
        return nullptr;
    }

    EnumDecl* ModDecl::getEnum(const std::string& name) {
        for (auto&& en: enums) {
            if (en->_name == name) {
                return en;
            }
        }
        return nullptr;
    }

    std::vector<FuncDecl*> ModDecl::getFunctions(const std::string& name) {
        std::vector<FuncDecl*> funcs;
        for (auto&& function: functions) {
            if (function->name == name) {
                funcs.push_back(function);
            }
        }
        return funcs;
    }

    void ModDecl::addFunction(FuncDecl* func) {
        functions.push_back(func);
    }

    void ModDecl::addClass(ClassDecl* cls) {
        classes.push_back(cls);
    }
}