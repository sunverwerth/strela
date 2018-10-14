// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_Ast_Token_h
#define Strela_Ast_Token_h

#include <map>
#include <string>

namespace Strela {
    #define TOKENS(X) \
        X(Amp) \
        X(AmpAmp) \
        X(AmpEquals) \
        X(As) \
        X(Asterisk) \
        X(AsteriskEquals) \
        X(Boolean) \
        X(BracketClose) \
        X(BracketOpen) \
        X(Caret) \
        X(CaretEquals) \
        X(Class) \
        X(Colon) \
        X(Comma) \
        X(CurlyClose) \
        X(CurlyOpen) \
        X(Else) \
        X(Enum) \
        X(Eof) \
        X(Equals) \
        X(EqualsEquals) \
        X(ExclamationMark) \
        X(ExclamationMarkEquals) \
        X(Export) \
        X(External) \
        X(Float) \
        X(Function) \
        X(GreaterThan) \
        X(GreaterThanEquals) \
        X(Identifier) \
        X(If) \
        X(Import) \
        X(Integer) \
        X(Interface) \
        X(Invalid) \
        X(Is) \
        X(LessThan) \
        X(LessThanEquals) \
        X(Minus) \
        X(MinusEquals) \
        X(MinusMinus) \
        X(Module) \
        X(Mutable) \
        X(New) \
        X(Null) \
        X(ParenClose) \
        X(ParenOpen) \
        X(Percent) \
        X(PercentEquals) \
        X(Period) \
        X(Pipe) \
        X(PipeEquals) \
        X(PipePipe) \
        X(Plus) \
        X(PlusEquals) \
        X(PlusPlus) \
        X(QuestionMark) \
        X(Return) \
        X(Semicolon) \
        X(Slash) \
        X(SlashEquals) \
        X(String) \
        X(This) \
        X(Tilde) \
        X(Type) \
        X(Var) \
        X(While) \


    #define AS_ENUM(X) X,
    enum class TokenType {
        TOKENS(AS_ENUM)
    };
    #undef AS_ENUM

    struct Token {
    public:
        Token() = default;
        Token(TokenType type, const std::string& trivia, const std::string& value, int line, int column, int index): type(type), trivia(trivia), value(value), line(line), column(column), index(index) {}
    public:
        TokenType type;
        std::string value;
        std::string trivia;
        int line;
        int column;
        int index;

        double floatVal();
        int64_t intVal();
        bool boolVal();
    };

    std::ostream& operator<<(std::ostream& stream, const Token& t);
    std::string getTokenName(TokenType type);
    std::string getTokenVal(TokenType type);
}

#endif