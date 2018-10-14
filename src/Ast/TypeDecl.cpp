#include "TypeDecl.h"
#include "ModDecl.h"

namespace Strela {
    std::string TypeDecl::getFullName() {
        ModDecl* mod = parent ? parent->as<ModDecl>() : nullptr;
        if (mod && !mod->_name.empty()) {
            return mod->getFullName() + "." + _name;
        }
        return _name;
    }
}