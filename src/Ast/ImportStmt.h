// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_AstImportStmt_h
#define Strela_Ast_AstImportStmt_h

#include "Stmt.h"

#include <string>
#include <vector>

namespace Strela {
    class ImportStmt: public Stmt {
    public:
        STRELA_GET_TYPE(Strela::ImportStmt, Strela::Stmt);
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
        ModDecl* module = nullptr;
    };
}

#endif