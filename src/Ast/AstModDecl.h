#ifndef Strela_Ast_AstModDecl_h
#define Strela_Ast_AstModDecl_h

#include "AstNode.h"
#include "../Types/ModuleType.h"

#include <vector>
#include <string>

namespace Strela {
    class AstFuncDecl;
    class AstClassDecl;
    class ModuleType;
    class AstEnumDecl;

    class AstModDecl: public AstNode {
    public:
        AstModDecl(
            const Token& startToken,
            const std::string& name,
            const std::vector<AstImportStmt*>& imports,
            const std::vector<AstFuncDecl*>& functions,
            const std::vector<AstClassDecl*>& classes,
            const std::vector<AstEnumDecl*>& enums
        ): AstNode(startToken), name(name), imports(imports), functions(functions), classes(classes), enums(enums), declType(new ModuleType(name, this)) {}

        STRELA_GET_TYPE(Strela::AstModDecl, Strela::AstNode);
        STRELA_IMPL_VISITOR;

        AstNode* getMember(const std::string& name);
        AstClassDecl* getClass(const std::string& name);
        std::vector<AstFuncDecl*> getFunctions(const std::string& name);
        void addFunction(AstFuncDecl* func);
        void addClass(AstClassDecl* cls);

    public:
        ModuleType* declType;
        std::string name;
        std::vector<AstImportStmt*> imports;
        std::vector<AstFuncDecl*> functions;
        std::vector<AstClassDecl*> classes;
        std::vector<AstEnumDecl*> enums;
        std::string filename;
    };
}

#endif