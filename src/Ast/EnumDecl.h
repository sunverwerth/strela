#ifndef Strela_Ast_AstEnumDecl_h
#define Strela_Ast_AstEnumDecl_h

#include "TypeDecl.h"

#include <vector>

namespace Strela {
    class EnumElement;
    
    class EnumDecl: public TypeDecl {
    public:
        EnumDecl(const std::string& name, const std::vector<EnumElement*>& elements): TypeDecl(name), elements(elements) {}
        STRELA_GET_TYPE(Strela::EnumDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name) override;

    public:
        std::vector<EnumElement*> elements;
        bool isExported = false;
    };
}

#endif