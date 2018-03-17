#ifndef Strela_Ast_AstModDecl_h
#define Strela_Ast_AstModDecl_h

#include "TypeDecl.h"

#include <vector>
#include <string>

namespace Strela {
    class FuncDecl;
    class ClassDecl;
    class ModuleType;
    class EnumDecl;
    class ImportStmt;

    class ModDecl: public TypeDecl {
    public:
        ModDecl(
            const Token& startToken,
            const std::string& name,
            const std::vector<ImportStmt*>& imports,
            const std::vector<FuncDecl*>& functions,
            const std::vector<ClassDecl*>& classes,
            const std::vector<EnumDecl*>& enums
        ): TypeDecl(startToken, name), imports(imports), functions(functions), classes(classes), enums(enums) {}

        STRELA_GET_TYPE(Strela::ModDecl, Strela::TypeDecl);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        ClassDecl* getClass(const std::string& name);
        std::vector<FuncDecl*> getFunctions(const std::string& name);
        void addFunction(FuncDecl* func);
        void addClass(ClassDecl* cls);

    public:
        std::vector<ImportStmt*> imports;
        std::vector<FuncDecl*> functions;
        std::vector<ClassDecl*> classes;
        std::vector<EnumDecl*> enums;
        std::string filename;
    };
}

#endif