#ifndef Strela_Ast_TypeDecl_h
#define Strela_Ast_TypeDecl_h

#include "Stmt.h"

#include <string>
#include <vector>

namespace Strela {
    class TypeDecl: public Stmt {
    public:
        TypeDecl(const Token& startToken, const std::string& name): Stmt(startToken), name(name) {}
        STRELA_GET_TYPE(Strela::TypeDecl, Strela::Stmt);
        virtual Node* getMember(const std::string& name) { return nullptr; }

    public:
        std::string name;
    };
}

#endif