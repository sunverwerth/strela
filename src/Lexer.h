// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Lexer_h
#define Strela_Lexer_h

#include "Ast/Token.h"
#include "Pass.h"

#include <iostream>
#include <vector>

namespace Strela {
    /**
     * Lexer for the strela language
     */
    class Lexer: public Pass {
    public:
        Lexer(std::istream& in): in(in), line(1), column(1) {}
        std::vector<Token> tokenize();

    private:
        int get();
        int peek();
        bool eof();
        bool match(char c);

    private:
        int numtokens = 0;
        std::istream& in;
        int line;
        int column;
        int ch;
    };
}

#endif