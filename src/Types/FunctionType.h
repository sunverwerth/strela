#ifndef Strela_Types_FunctionType_h
#define Strela_Types_FunctionType_h

#include "Type.h"

#include <vector>

namespace Strela {
    class AstFuncDecl;
    class FunctionType: public Type {
    public:
        FunctionType(const std::vector<Type*>& paramTypes, Type* returnType);
        STRELA_GET_TYPE(Strela::FunctionType, Strela::Type);

        bool isAssignableFrom(const Type* other) const override;
        bool isCallable(const std::vector<Type*>& argTypes) const override;

        static FunctionType* get(AstFuncDecl* func);

    public:
        std::vector<Type*> paramTypes;
        Type* returnType;

        static std::map<std::string, FunctionType*> functionTypes;
    };
}
#endif