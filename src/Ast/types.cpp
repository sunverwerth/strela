// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

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
#include "PointerType.h"
#include "Token.h"

namespace Strela {
    VoidType VoidType::instance;
    NullType NullType::instance;
    BoolType BoolType::instance;
    TypeType TypeType::instance;
    OverloadedFuncType OverloadedFuncType::instance;

    IntType IntType::u8("u8", false, 1);
    IntType IntType::u16("u16", false, 2);
    IntType IntType::u32("u32", false, 4);
    IntType IntType::u64("u64", false, 8);
    IntType IntType::i8("i8", true, 1);
    IntType IntType::i16("i16", true, 2);
    IntType IntType::i32("i32", true, 4);
    IntType IntType::i64("i64", true, 8);

    FloatType FloatType::f32("f32", 4);
    FloatType FloatType::f64("f64", 8);

    InvalidType InvalidType::instance;

    ClassDecl ClassDecl::String("String");

    PointerType PointerType::instance;

    std::map<TypeDecl*, ArrayType*> ArrayType::arrayTypes;
    std::vector<FuncType*> FuncType::funcTypes;
}