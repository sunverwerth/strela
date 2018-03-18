#ifndef Strela_SourceFile_h
#define Strela_SourceFile_h

#include "Ast/Token.h"

#include <string>
#include <vector>

namespace Strela {
    class SourceFile {
    public:
        SourceFile(const std::string& filename, const std::vector<Token>& tokens): filename(filename), tokens(tokens) {}
    
    public:
        std::string filename;
        std::vector<Token> tokens;
    };
}

#endif