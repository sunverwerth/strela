#ifndef Strela_Ast_AstImportStmt_h
#define Strela_Ast_AstImportStmt_h

#include "AstStmt.h"

#include <string>
#include <vector>

namespace Strela {
    class AstImportStmt: public AstStmt {
    public:
        AstImportStmt(const Token& startToken, const std::vector<std::string>& parts, bool all): AstStmt(startToken), parts(parts), all(all) {}
        STRELA_GET_TYPE(Strela::AstImportStmt, Strela::AstStmt);
        STRELA_IMPL_STMT_VISITOR;

        std::string getFullName(const char* separator = ".") {
            std::string result;
            for (size_t i = 0; i < parts.size(); ++i) {
                result += parts[i];
                if (i < parts.size() - 1) {
                    result += separator;
                }
            }
            return result;
        }

        std::string getBaseName(const char* separator = ".") {
            std::string result;
            for (size_t i = 0; i < parts.size() - 1; ++i) {
                result += parts[i];
                if (i < parts.size() - 2) {
                    result += separator;
                }
            }
            return result;
        }

    public:
        std::vector<std::string> parts;
        bool all;
        bool importModule = false;
        AstModDecl* module = nullptr;
    };
}

#endif