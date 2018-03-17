#include "VoidType.h"
#include "BoolType.h"
#include "NullType.h"
#include "VoidType.h"
#include "TypeType.h"
#include "OverloadedFuncType.h"
#include "IntType.h"
#include "FloatType.h"
#include "FuncType.h"
#include "ClassDecl.h"
#include "InvalidType.h"
#include "ArrayType.h"
#include "Token.h"

namespace Strela {
    VoidType VoidType::instance(Token(TokenType::Identifier, "", 0, 0));
    NullType NullType::instance(Token(TokenType::Identifier, "", 0, 0));
    BoolType BoolType::instance(Token(TokenType::Identifier, "", 0, 0));
    TypeType TypeType::instance(Token(TokenType::Identifier, "", 0, 0));
    OverloadedFuncType OverloadedFuncType::instance(Token(TokenType::Identifier, "", 0, 0));

    IntType IntType::instance(Token(TokenType::Identifier, "", 0, 0), "int", false, 8);

    FloatType FloatType::instance(Token(TokenType::Identifier, "", 0, 0), "float", 8);

    InvalidType InvalidType::instance(Token(TokenType::Identifier, "", 0, 0));

    ClassDecl ClassDecl::String(Token(TokenType::Identifier, "", 0, 0), "String", {}, {});

    std::map<TypeDecl*, ArrayType*> ArrayType::arrayTypes;
    std::vector<FuncType*> FuncType::funcTypes;
}