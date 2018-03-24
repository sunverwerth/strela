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
#include "Token.h"

namespace Strela {
    VoidType VoidType::instance;
    NullType NullType::instance;
    BoolType BoolType::instance;
    TypeType TypeType::instance;
    OverloadedFuncType OverloadedFuncType::instance;

    IntType IntType::instance("int", false, 8);

    FloatType FloatType::instance("float", 8);

    InvalidType InvalidType::instance;

    ClassDecl ClassDecl::String("String", {}, {}, {});

    std::map<TypeDecl*, ArrayType*> ArrayType::arrayTypes;
    std::vector<FuncType*> FuncType::funcTypes;
}