#ifndef Strela_Ast_AstModDecl_h
#define Strela_Ast_AstModDecl_h

#include "Stmt.h"
#include "../Types/ModuleType.h"

#include <vector>
#include <string>

namespace Strela {
    class FuncDecl;
    class ClassDecl;
    class ModuleType;
    class EnumDecl;
    class ImportStmt;

    class ModDecl: public Stmt {
    public:
        ModDecl(
            const Token& startToken,
            const std::string& name,
            const std::vector<ImportStmt*>& imports,
            const std::vector<FuncDecl*>& functions,
            const std::vector<ClassDecl*>& classes,
            const std::vector<EnumDecl*>& enums
        ): Stmt(startToken), name(name), imports(imports), functions(functions), classes(classes), enums(enums), declType(new ModuleType(name, this)) {}

        STRELA_GET_TYPE(Strela::ModDecl, Strela::Stmt);
        STRELA_IMPL_STMT_VISITOR;

        Node* getMember(const std::string& name);
        ClassDecl* getClass(const std::string& name);
        std::vector<FuncDecl*> getFunctions(const std::string& name);
        void addFunction(FuncDecl* func);
        void addClass(ClassDecl* cls);

    public:
        ModuleType* declType;
        std::string name;
        std::vector<ImportStmt*> imports;
        std::vector<FuncDecl*> functions;
        std::vector<ClassDecl*> classes;
        std::vector<EnumDecl*> enums;
        std::string filename;
    };
}

#endif