#include "Lexer.h"

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
        { "var", TokenType::Var },
        { "export", TokenType::Export },
        { "import", TokenType::Import },
        { "class", TokenType::Class },
        { "interface", TokenType::Interface },
        { "module", TokenType::Module },
        { "function", TokenType::Function },
        { "true", TokenType::Boolean },
        { "false", TokenType::Boolean },
        { "null", TokenType::Null },
        { "return", TokenType::Return },
        { "new", TokenType::New },
        { "mutable", TokenType::Mutable },
        { "if", TokenType::If },
        { "else", TokenType::Else },
        { "while", TokenType::While },
    };

    std::string unescape(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\\') {
                if (i < str.size() - 1) {
                    char c = str[++i];
                    switch (c) {
                        case '0': unescaped += '0'; break;
                        case 'r': unescaped += '\t'; break;
                        case 'n': unescaped += '\n'; break;
                        case 't': unescaped += '\t'; break;
                        case '\\': unescaped += '\\'; break;
                        default:
                        unescaped += '\\';
                        unescaped += c;
                    }
                    continue;
                }
            }
            unescaped += str[i];
        }
        return unescaped;
    }

    std::string escape(const std::string& str) {
        std::string escaped;
        for (size_t i = 0; i < str.size(); ++i) {
            switch (str[i]) {
                case '0': escaped += "\\0"; break;
                case '\r': escaped += "\\t"; break;
                case '\n': escaped += "\\n"; break;
                case '\t': escaped += "\\t"; break;
                case '\\': escaped += "\\\\"; break;
                default:
                escaped += str[i];
            }
        }
        return escaped;
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        if (in.good()) {
            std::stringstream sstr;
            get();
            while (!eof()) {
                while (!eof()) {
                    while (iswspace(ch) || match('\r') || match('\n')) {
                        get();
                    }

                    if (match('/')) {
                        get();
                        if (match('/')) {
                            get();
                            while (!eof() && !match('\n')) {
                                get();
                            }
                        }
                        else if (match('*')) {
                            get();
                            while (!eof()) {
                                int ch2 = ch;
                                get();
                                if (ch2 == '*' && match('/')) {
                                    get();
                                    break;
                                }
                                get();
                            }
                        }
                        else {
                            tokens.push_back(Token(TokenType::Slash, "/", line, column));
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }

                int startLine = line;
                int startColumn = column;

                if (eof()) {
                    tokens.push_back(Token(TokenType::Eof, "", startLine, startColumn));
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
                        tokens.push_back(Token(TokenType::Float, sstr.str(), startLine, startColumn));
                    }
                    else {
                        tokens.push_back(Token(TokenType::Integer, sstr.str(), startLine, startColumn));
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
                    tokens.push_back(Token(ttype, sstr.str(), startLine, startColumn));
                }
                else if (match('"')) {
                    std::stringstream sstr;
                    get();
                    while (!eof() && !match('"')) {
                        sstr << (char)ch;
                        get();
                    }
                    get();
                    tokens.push_back(Token(TokenType::String, unescape(sstr.str()), startLine, startColumn));
                }
                else {
                    int ch2 = ch;
                    get();
                    std::string two(1, (char)ch2);
                    two += (char)ch;

                    auto it2 = twoCharTokens.find(two);
                    if (it2 != twoCharTokens.end()) {
                        tokens.push_back(Token(it2->second, two, startLine, startColumn));
                        get();
                    }
                    else {
                        auto it = singleCharTokens.find((char)ch2);
                        if (it != singleCharTokens.end()) {
                            tokens.push_back(Token(it->second, std::string(1, (char)ch2), startLine, startColumn));
                        }
                        else {
                            tokens.push_back(Token(TokenType::Invalid, std::string(1, (char)ch2), startLine, startColumn));
                        }
                    }
                }
            }
        }
        else {
            tokens.push_back(Token(TokenType::Eof, "", 0, 0));
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
    
    bool Lexer::eof() {
        return in.eof();
    }

    bool Lexer::match(char c) {
        return ch == c;
    }
}