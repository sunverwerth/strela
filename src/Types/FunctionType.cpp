#include "FunctionType.h"
#include "../Ast/nodes.h"

#include <sstream>
#include <string>

namespace Strela {
    std::string makeSignature(const std::vector<Type*>& paramTypes, Type* returnTypeExpr) {
        std::stringstream sstr;
        sstr << "function(";
        for(auto&& param: paramTypes) {
            sstr << param->name;
            if (&param != &paramTypes.back()) {
                sstr << ",";
            }
        }
        sstr << "):" << returnTypeExpr->name;
        return sstr.str();
    }

    FunctionType::FunctionType(const std::vector<Type*>& paramTypes, Type* returnType): Type(makeSignature(paramTypes, returnType)), returnType(returnType), paramTypes(paramTypes) {}

    bool FunctionType::isAssignableFrom(const Type* other) const {
        return other == this;
    }

    FunctionType* FunctionType::get(AstFuncDecl* function) {
        auto returnTypeExpr = function->returnTypeExpr ? function->returnTypeExpr->staticValue.type : Types::_void;
        std::vector<Type*> paramTypes;
        for (auto&& param: function->params) {
            paramTypes.push_back(param->declType);
        }

        auto sign = makeSignature(paramTypes, returnTypeExpr);
        auto it = functionTypes.find(sign);
        if (it != functionTypes.end()) return it->second;
        auto ft = new FunctionType(paramTypes, returnTypeExpr);
        functionTypes[sign] = ft;
        return ft;
    }

    bool FunctionType::isCallable(const std::vector<Type*>& argTypes) const {
        if (argTypes.size() != paramTypes.size()) {
            return false;
        }
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (!paramTypes[i]->isAssignableFrom(argTypes[i])) {
                return false;
            }
        }
        return true;
    }
    
    std::map<std::string, FunctionType*> FunctionType::functionTypes;
}