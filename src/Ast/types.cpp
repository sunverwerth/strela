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

    IntType IntType::u8(Token(TokenType::Identifier, "", 0, 0), "u8", false, 1);
    IntType IntType::u16(Token(TokenType::Identifier, "", 0, 0), "u16", false, 2);
    IntType IntType::u32(Token(TokenType::Identifier, "", 0, 0), "u32", false, 4);
    IntType IntType::u64(Token(TokenType::Identifier, "", 0, 0), "u64", false, 8);
    IntType IntType::i8(Token(TokenType::Identifier, "", 0, 0), "i8", true, 1);
    IntType IntType::i16(Token(TokenType::Identifier, "", 0, 0), "i16", true, 2);
    IntType IntType::i32(Token(TokenType::Identifier, "", 0, 0), "i32", true, 4);
    IntType IntType::i64(Token(TokenType::Identifier, "", 0, 0), "i64", true, 8);

    FloatType FloatType::f32(Token(TokenType::Identifier, "", 0, 0), "f32", 4);
    FloatType FloatType::f64(Token(TokenType::Identifier, "", 0, 0), "f64", 8);

    InvalidType InvalidType::instance(Token(TokenType::Identifier, "", 0, 0));

    ClassDecl ClassDecl::String(Token(TokenType::Identifier, "", 0, 0), "String", {}, {});

    std::map<TypeDecl*, ArrayType*> ArrayType::arrayTypes;
    std::vector<FuncType*> FuncType::funcTypes;
}