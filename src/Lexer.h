#ifndef Strela_Lexer_h
#define Strela_Lexer_h

#include "Ast/Token.h"

#include <iostream>
#include <vector>

namespace Strela {
    /**
     * Lexer for the strela language
     */
    class Lexer {
    public:
        Lexer(std::istream& in): in(in), line(1), column(1) {}
        std::vector<Token> tokenize();

    private:
        int get();
        bool eof();
        bool match(char c);

    private:
        std::istream& in;
        int line;
        int column;
        int ch;
    };
}

#endif