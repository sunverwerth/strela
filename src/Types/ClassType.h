#ifndef Strela_Types_ClassType_h
#define Strela_Types_ClassType_h

#include "Type.h"

namespace Strela {
    class ClassDecl;
    class ClassType: public Type {
    public:
        ClassType(const std::string& name, ClassDecl* _class): Type(name), _class(_class) {}
        STRELA_GET_TYPE(Strela::ClassType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;

    public:
        ClassDecl* _class;
    };
}
#endif