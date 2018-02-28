#ifndef Strela_Types_ModuleType_h
#define Strela_Types_ModuleType_h

#include "Type.h"

namespace Strela {
    class AstModDecl;

    class ModuleType: public Type {
    public:
        ModuleType(const std::string& name, AstModDecl* module): Type(name), module(module) {}
        STRELA_GET_TYPE(Strela::ModuleType, Strela::Type);

    public:
        AstModDecl* module;
    };
}
#endif