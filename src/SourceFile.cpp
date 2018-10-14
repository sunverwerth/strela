#include "SourceFile.h"
#include <sstream>

namespace Strela {
    std::string SourceFile::getLine(int line) const {
        auto sstr = std::stringstream();
        for (auto& tok: tokens) {
            if (tok.line > line) break;
            if (tok.line == line) sstr << tok.trivia << tok.value;
        }
        return sstr.str();
    }
}