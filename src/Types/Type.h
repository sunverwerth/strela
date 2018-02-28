#ifndef Strela_Types_Type_h
#define Strela_Types_Type_h

#include "../TypeInfo.h"

#include <string>
#include <vector>

namespace Strela {
    class Type {
    public:
        Type(const std::string& name): name(name) {}
        STRELA_BASE_TYPE(Strela::Type);

        virtual bool isAssignableFrom(const Type* other) const { return false; }
        virtual bool isScalar() const { return false; }
        virtual bool isCallable(const std::vector<Type*>&) const { return false; }

    public:
        std::string name;
    };
}
#endif