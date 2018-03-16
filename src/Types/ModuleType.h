#ifndef Strela_Types_ModuleType_h
#define Strela_Types_ModuleType_h

#include "Type.h"

namespace Strela {
    class ModDecl;

    class ModuleType: public Type {
    public:
        ModuleType(const std::string& name, ModDecl* module): Type(name), module(module) {}
        STRELA_GET_TYPE(Strela::ModuleType, Strela::Type);

    public:
        ModDecl* module;
    };
}
#endif