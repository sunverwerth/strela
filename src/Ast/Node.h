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
        int line = 0;
        int column = 0;
        const SourceFile* source = nullptr;
    };
}
#endif