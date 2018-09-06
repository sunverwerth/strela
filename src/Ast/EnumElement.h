// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstEnumElement_h
#define Strela_Ast_AstEnumElement_h

#include "Stmt.h"

#include <string>

namespace Strela {    
    class EnumElement: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::EnumElement, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        int index = 0;
    };
}

#endif