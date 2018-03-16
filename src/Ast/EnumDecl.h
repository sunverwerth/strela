#ifndef Strela_Ast_AstEnumDecl_h
#define Strela_Ast_AstEnumDecl_h

#include "Stmt.h"
#include "../Types/types.h"

#include <vector>
#include <string>

namespace Strela {
    class EnumElement;
    
    class EnumDecl: public Stmt {
    public:
        EnumDecl(const Token& startToken, const std::string& name, const std::vector<EnumElement*>& elements): Stmt(startToken), name(name), elements(elements), declType(new EnumType(this)) {}
        STRELA_GET_TYPE(Strela::EnumDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

        EnumElement* getMember(const std::string& name);

    public:
        std::string name;
        std::vector<EnumElement*> elements;
        bool isExported = false;
        Type* declType;
    };
}

#endif