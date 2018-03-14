#ifndef Strela_Ast_AstEnumElement_h
#define Strela_Ast_AstEnumElement_h

#include "AstStmt.h"

#include <string>

namespace Strela {    
    class AstEnumElement: public AstStmt {
    public:
        AstEnumElement(const Token& startToken): AstStmt(startToken), name(startToken.value) {}
        STRELA_GET_TYPE(Strela::AstEnumElement, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

    public:
        std::string name;
    };
}

#endif