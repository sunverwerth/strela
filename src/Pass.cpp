// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "Pass.h"
#include "Ast/Node.h"
#include "Ast/Token.h"
#include "SourceFile.h"

#include <iostream>

namespace Strela {

    bool Pass::hadErrors() {
        return hasErrors;
    }

    bool Pass::hadWarnings() {
        return hasWarnings;
    }

    void Pass::info(const std::string& msg) {
        std::cerr << "\033[1;34m" << msg << "\033[0m\n";
    }

    void Pass::info(const Node& n, const std::string& msg) {
        std::cerr << "\033[1;34m";
        if (n.source) {
            std::cerr << n.source->filename << ":" << n.line << ":" << n.column << " ";
        }
        std::cerr << msg << "\033[0m\n";
    }

    void Pass::info(const SourceFile& file, const Token& n, const std::string& msg) {
        std::cerr << "\033[1;34m" << file.filename << ":"  << n.line << ":" << n.column << " " << msg << "\033[0m\n";
    }

    void Pass::warning(const std::string& msg) {
        hasWarnings = true;
        std::cerr << "\033[1;33mWarning: " << msg << "\033[0m\n";
    }

    void Pass::warning(const Node& n, const std::string& msg) {
        hasWarnings = true;
        std::cerr << "\033[1;33m";
        if (n.source) {
            std::cerr << n.source->filename << ":" << n.line << ":" << n.column << " ";
        }
        std::cerr << "Warning: " << msg << "\033[0m\n";
    }

    void Pass::warning(const SourceFile& file, const Token& n, const std::string& msg) {
        hasWarnings = true;
        std::cerr << "\033[1;33m" << file.filename << ":"  << n.line << ":" << n.column << " Warning: " << msg << "\033[0m\n";
    }

    void Pass::error(const std::string& msg) {
        hasErrors = true;
        std::cerr << "\033[1;31mError: " << msg << "\033[0m\n";
    }

    void Pass::error(const Node& n, const std::string& msg) {
        hasErrors = true;
        std::cerr << "\033[1;31m";
        if (n.source) {
            std::cerr << n.source->filename << ":" << n.line << ":" << n.column << " ";
        }
        std::cerr << "Error: " << msg << "\033[0m\n";
    }

    void Pass::error(const SourceFile& file, const Token& n, const std::string& msg) {
        hasErrors = true;
        std::cerr << "\033[1;31m" << file.filename << ":" << n.line << ":" << n.column << " Error: " << msg << "\033[0m\n";
    }
}