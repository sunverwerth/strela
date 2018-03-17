#ifndef Strela_Ast_AstEnumDecl_h
#define Strela_Ast_AstEnumDecl_h

#include "TypeDecl.h"

#include <vector>

namespace Strela {
    class EnumElement;
    
    class EnumDecl: public TypeDecl {
    public:
        EnumDecl(const Token& startToken, const std::string& name, const std::vector<EnumElement*>& elements): TypeDecl(startToken, name), elements(elements) {}
        STRELA_GET_TYPE(Strela::EnumDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        EnumElement* getMember(const std::string& name);

    public:
        std::vector<EnumElement*> elements;
        bool isExported = false;
    };
}

#endif