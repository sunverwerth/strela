#ifndef Strela_Ast_Token_h
#define Strela_Ast_Token_h

#include <map>
#include <string>

namespace Strela {
    #define TOKENS(X) \
        X(Amp) \
        X(AmpAmp) \
        X(AmpEquals) \
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
        X(Eof) \
        X(Equals) \
        X(EqualsEquals) \
        X(ExclamationMark) \
        X(Export) \
        X(Import) \
        X(ExclamationMarkEquals) \
        X(Float) \
        X(Function) \
        X(GreaterThan) \
        X(GreaterThanEquals) \
        X(Identifier) \
        X(If) \
        X(Integer) \
        X(Interface) \
        X(Invalid) \
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
        X(Tilde) \
        X(Var) \
        X(While) \
        X(Enum) \
        X(This) \


    #define AS_ENUM(X) X,
    enum class TokenType {
        TOKENS(AS_ENUM)
    };
    #undef AS_ENUM

    struct Token {
    public:
        Token(TokenType type, const std::string& value, int line, int column): type(type), value(value), line(line), column(column) {}
    public:
        TokenType type;
        std::string value;
        int line;
        int column;

        double floatVal();
        int64_t intVal();
        bool boolVal();
    };

    std::ostream& operator<<(std::ostream& stream, const Token& t);
    std::string getTokenName(TokenType type);
    std::string getTokenVal(TokenType type);
}

#endif