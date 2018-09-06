// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstNode_h
#define Strela_Ast_AstNode_h

#include "../TypeInfo.h"
#include "Token.h"

#include <iostream>

namespace Strela {
    class SourceFile;
    class Node {
    public:
        Node() {}
        virtual ~Node() {}
        STRELA_BASE_TYPE(Strela::Node);

    public:
        Node* parent = nullptr;
        int line = 0;
		int lineend = 0;
		int column = 0;
        const SourceFile* source = nullptr;
        int firstToken = 0;
    };
}
#endif