#ifndef Strela_Ast_AstEnumDecl_h
#define Strela_Ast_AstEnumDecl_h

#include "AstStmt.h"
#include "../Types/types.h"

#include <vector>
#include <string>

namespace Strela {
    class AstEnumElement;
    
    class AstEnumDecl: public AstStmt {
    public:
        AstEnumDecl(const Token& startToken, const std::string& name, const std::vector<AstEnumElement*>& elements): AstStmt(startToken), name(name), elements(elements), declType(new EnumType(this)) {}
        STRELA_GET_TYPE(Strela::AstEnumDecl, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

        AstEnumElement* getMember(const std::string& name);

    public:
        std::string name;
        std::vector<AstEnumElement*> elements;
        bool isExported = false;
        Type* declType;
    };
}

#endif