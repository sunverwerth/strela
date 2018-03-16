#ifndef Strela_Ast_AstEnumElement_h
#define Strela_Ast_AstEnumElement_h

#include "Stmt.h"

#include <string>

namespace Strela {    
    class EnumElement: public Stmt {
    public:
        EnumElement(const Token& startToken): Stmt(startToken), name(startToken.value) {}
        STRELA_GET_TYPE(Strela::EnumElement, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
        int index = 0;
    };
}

#endif