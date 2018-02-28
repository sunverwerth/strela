#ifndef Strela_Types_types_h
#define Strela_Types_types_h

#include "Type.h"
#include "IntType.h"
#include "BoolType.h"
#include "FunctionType.h"
#include "VoidType.h"
#include "FloatType.h"
#include "NullType.h"
#include "TypeType.h"
#include "ClassType.h"
#include "ModuleType.h"
#include "InvalidType.h"
#include "ArrayType.h"

namespace Strela {
    namespace Types {
        extern Type* i8;
        extern Type* i16;
        extern Type* i32;
        extern Type* i64;
        extern Type* u8;
        extern Type* u16;
        extern Type* u32;
        extern Type* u64;
        extern Type* boolean;
        extern Type* _void;
        extern Type* f32;
        extern Type* f64;
        extern Type* null;
        extern Type* typetype;
        extern Type* string;
        extern Type* invalid;
    }
}

#endif