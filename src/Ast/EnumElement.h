// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstEnumElement_h
#define Strela_Ast_AstEnumElement_h

#include "Node.h"

#include <string>

namespace Strela {    
    class EnumElement: public Node {
    public:
        STRELA_GET_TYPE(Strela::EnumElement, Strela::Node);

    public:
        std::string name;
        int index = 0;
    };
}

#endif