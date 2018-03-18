#include "Token.h"

#include <iostream>
#include <string>

namespace Strela {
    #define AS_PAIR(X) {TokenType::X, #X},
    std::map<TokenType, std::string> tokenNames {
        TOKENS(AS_PAIR)
    };
    #undef AS_PAIR

    std::ostream& operator<<(std::ostream& stream, const Token& t) {
        return stream << tokenNames[t.type] << " '" << t.value << "'";
    }

    std::string getTokenName(TokenType type) {
        return tokenNames[type];
    }

    std::string getTokenVal(TokenType type) {
        switch (type) {
            case TokenType::Amp: return "&";
            case TokenType::AmpAmp: return "&&";
            case TokenType::AmpEquals: return "&=";
            case TokenType::Asterisk: return "*";
            case TokenType::AsteriskEquals: return "*=";
            case TokenType::BracketClose: return "]";
            case TokenType::BracketOpen: return "[";
            case TokenType::Caret: return "^";
            case TokenType::CaretEquals: return "^=";
            case TokenType::Class: return "class";
            case TokenType::Colon: return ":";
            case TokenType::Comma: return ",";
            case TokenType::CurlyClose: return "}";
            case TokenType::CurlyOpen: return "{";
            case TokenType::Else: return "else";
            case TokenType::Equals: return "=";
            case TokenType::EqualsEquals: return "==";
            case TokenType::ExclamationMark: return "!";
            case TokenType::Export: return "export";
            case TokenType::Import: return "import";
            case TokenType::ExclamationMarkEquals: return "!=";
            case TokenType::Function: return "function";
            case TokenType::GreaterThan: return ">";
            case TokenType::GreaterThanEquals: return ">=";
            case TokenType::If: return "if";
            case TokenType::Interface: return "interface";
            case TokenType::LessThan: return "<";
            case TokenType::LessThanEquals: return "<=";
            case TokenType::Minus: return "-";
            case TokenType::MinusEquals: return "-=";
            case TokenType::MinusMinus: return "--";
            case TokenType::Module: return "module";
            case TokenType::Mutable: return "mutable";
            case TokenType::New: return "new";
            case TokenType::Null: return "null";
            case TokenType::ParenClose: return ")";
            case TokenType::ParenOpen: return "(";
            case TokenType::Percent: return "%";
            case TokenType::PercentEquals: return "%=";
            case TokenType::Period: return ".";
            case TokenType::Pipe: return "|";
            case TokenType::PipeEquals: return "|=";
            case TokenType::PipePipe: return "||";
            case TokenType::Plus: return "+";
            case TokenType::PlusEquals: return "+=";
            case TokenType::PlusPlus: return "++";
            case TokenType::QuestionMark: return "?";
            case TokenType::Return: return "return";
            case TokenType::Semicolon: return ";";
            case TokenType::Slash: return "/";
            case TokenType::SlashEquals: return "/=";
            case TokenType::Tilde: return "~";
            case TokenType::Var: return "var";
            case TokenType::While: return "while";
            case TokenType::Enum: return "enum";
            default: return "";
        }
    }

    double Token::floatVal() {
        return std::stod(value);
    }

    int64_t Token::intVal() {
        return std::stoll(value);
    }

    bool Token::boolVal() {
        return value == "true";
    }
}