// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "Lexer.h"
#include "utils.h"

#include <sstream>

namespace Strela {
    std::map<char, TokenType> singleCharTokens {
        { '+', TokenType::Plus },
        { '-', TokenType::Minus },
        { '*', TokenType::Asterisk },
        { '/', TokenType::Slash },
        { '!', TokenType::ExclamationMark },
        { '%', TokenType::Percent },
        { '&', TokenType::Amp },
        { '(', TokenType::ParenOpen },
        { ')', TokenType::ParenClose },
        { '=', TokenType::Equals },
        { '?', TokenType::QuestionMark },
        { '[', TokenType::BracketOpen },
        { ']', TokenType::BracketClose },
        { '{', TokenType::CurlyOpen },
        { '}', TokenType::CurlyClose },
        { '<', TokenType::LessThan },
        { '>', TokenType::GreaterThan },
        { '^', TokenType::Caret },
        { '~', TokenType::Tilde },
        { ',', TokenType::Comma },
        { ';', TokenType::Semicolon },
        { '.', TokenType::Period },
        { ':', TokenType::Colon },
        { '|', TokenType::Pipe },
    };

    std::map<std::string, TokenType> twoCharTokens {
        { "+=", TokenType::PlusEquals },
        { "-=", TokenType::MinusEquals },
        { "*=", TokenType::AsteriskEquals },
        { "/=", TokenType::SlashEquals },
        { "%=", TokenType::PercentEquals },
        { "==", TokenType::EqualsEquals },
        { "!=", TokenType::ExclamationMarkEquals },
        { "&&", TokenType::AmpAmp },
        { "||", TokenType::PipePipe },
        { "&=", TokenType::AmpEquals },
        { "|=", TokenType::PipeEquals },
        { "^=", TokenType::CaretEquals },
        { "++", TokenType::PlusPlus },
        { "--", TokenType::MinusMinus },
        { ">=", TokenType::GreaterThanEquals },
        { "<=", TokenType::LessThanEquals },
    };

    std::map<std::string, TokenType> keywords {
        { "class", TokenType::Class },
        { "else", TokenType::Else },
        { "enum", TokenType::Enum },
        { "export", TokenType::Export },
        { "external", TokenType::External },
        { "false", TokenType::Boolean },
        { "function", TokenType::Function },
        { "if", TokenType::If },
        { "import", TokenType::Import },
        { "interface", TokenType::Interface },
        { "is", TokenType::Is },
        { "as", TokenType::As },
        { "module", TokenType::Module },
        { "mutable", TokenType::Mutable },
        { "new", TokenType::New },
        { "null", TokenType::Null },
        { "return", TokenType::Return },
        { "this", TokenType::This },
        { "true", TokenType::Boolean },
        { "type", TokenType::Type },
        { "var", TokenType::Var },
        { "while", TokenType::While },
    };

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        if (in.good()) {
            std::stringstream sstr;
            get();
            while (!eof()) {
                std::string trivia;
                while (!eof()) {
                    while (iswspace(ch) || match('\r') || match('\n')) {
                        trivia += ch;
                        get();
                    }

                    auto next = peek();
                    if (match('/') && next == '/') {
                        trivia += ch;
                        get();
                        trivia += ch;
                        get();
                        while (!eof() && !match('\n')) {
                        trivia += ch;
                            get();
                        }
                        trivia += ch;
                        get();
                    }
                    else if (match('/') && next == '*') {
                        trivia += ch;
                        get();
                        trivia += ch;
                        get();
                        while (!eof()) {
                            int ch2 = ch;
                            trivia += ch;
                            get();
                            if (ch2 == '*' && match('/')) {
                                trivia += ch;
                                get();
                                break;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }

                int startLine = line;
                int startColumn = column;

                if (eof()) {
                    tokens.push_back(Token(TokenType::Eof, trivia, "", startLine, startColumn, numtokens++));
                    break;
                }
                else if (isdigit(ch)) {
                    std::stringstream sstr;
                    sstr << (char)ch;
                    get();
                    while (isdigit(ch)) {
                        sstr << (char)ch;
                        get();
                    }
                    if (match('.')) {
                        sstr << '.';
                        get();
                        while (isdigit(ch)) {
                            sstr << (char)ch;
                            get();
                        }
                        tokens.push_back(Token(TokenType::Float, trivia, sstr.str(), startLine, startColumn, numtokens++));
                    }
                    else {
                        tokens.push_back(Token(TokenType::Integer, trivia, sstr.str(), startLine, startColumn, numtokens++));
                    }
                }
                else if (isalpha(ch) || match('_')) {
                    std::stringstream sstr;
                    sstr << (char)ch;
                    get();
                    while(isalnum(ch) || match('_')) {
                        sstr << (char)ch;
                        get();
                    }
                    TokenType ttype(TokenType::Identifier);
                    auto it = keywords.find(sstr.str());
                    if (it != keywords.end()) {
                        ttype = it->second;
                    }
                    tokens.push_back(Token(ttype, trivia, sstr.str(), startLine, startColumn, numtokens++));
                }
                else if (match('"')) {
                    std::stringstream sstr;
                    get();
                    while (!eof() && !match('"')) {
                        if (ch == '\\') {
                            sstr << (char)ch;
                            get();
                        }
                        sstr << (char)ch;
                        get();
                    }
                    get();
                    tokens.push_back(Token(TokenType::String, trivia, unescape(sstr.str()), startLine, startColumn, numtokens++));
                }
                else {
                    int ch2 = ch;
                    get();
                    std::string two(1, (char)ch2);
                    two += (char)ch;

                    auto it2 = twoCharTokens.find(two);
                    if (it2 != twoCharTokens.end()) {
                        tokens.push_back(Token(it2->second, trivia, two, startLine, startColumn, numtokens++));
                        get();
                    }
                    else {
                        auto it = singleCharTokens.find((char)ch2);
                        if (it != singleCharTokens.end()) {
                            tokens.push_back(Token(it->second, trivia, std::string(1, (char)ch2), startLine, startColumn, numtokens++));
                        }
                        else {
                            tokens.push_back(Token(TokenType::Invalid, trivia, std::string(1, (char)ch2), startLine, startColumn, numtokens++));
                        }
                    }
                }
            }
        }
        else {
            tokens.push_back(Token(TokenType::Eof, "", "", 0, 0, numtokens++));
        }

        return tokens;
    }

    int Lexer::get() {
        ch = in.get();
        static int lastch = ch;

        if (lastch == '\n') {
            column = 1;
            line++;
        }
        else {
            column++;
        }

        lastch = ch;
        return ch;
    }

    int Lexer::peek() {
        return in.peek();
    }
    
    bool Lexer::eof() {
        return ch == EOF;
    }

    bool Lexer::match(char c) {
        return ch == c;
    }
}