#ifndef Strela_exceptions_h
#define Strela_exceptions_h

#include <stdexcept>
#include "Ast/Token.h"
#include "Ast/AstNode.h"

namespace Strela {

    class Exception: public std::runtime_error {
    public:
        Exception(const std::string& msg): std::runtime_error(msg) {}
    };

    class LexerException: public Exception {
    public:
        LexerException(const std::string& msg): Exception(msg) {}
    };

    class InvalidTokenException: public LexerException {
    public:
        InvalidTokenException(): LexerException("Invalid Token") {}
    };

    class ParseException: public Exception {
    public:
        ParseException(int line, int column, const std::string& msg): Exception(std::to_string(line) + ":" + std::to_string(column) + " " + msg) {}
    };

    class MissingTokenException: public ParseException {
    public:
        MissingTokenException(TokenType type, int line, int column): ParseException(line, column, "Missing Token " + getTokenName(type)) {}
    };

    class UnexpectedTokenException: public ParseException {
    public:
        UnexpectedTokenException(const Token& token, TokenType expectedType): ParseException(token.line, token.column, "Unexpected Token " + getTokenName(token.type) + ", expected " + getTokenName(expectedType)) {}
        UnexpectedTokenException(const Token& token, const std::string& expected): ParseException(token.line, token.column, "Unexpected Token " + getTokenName(token.type) + ", expected " + expected) {}
    };

    class DuplicateSymbolException: public Exception {
    public:
        DuplicateSymbolException(const std::string& name, const AstNode& n): Exception(std::to_string(n.startToken.line) + ":" + std::to_string(n.startToken.column) + " Duplicate symbol " + name) {}
        DuplicateSymbolException(const std::string& name): Exception("Duplicate symbol " + name) {}
    };

    class UnresolvedSymbolException: public Exception {
    public:
        UnresolvedSymbolException(const std::string& name, const AstNode& n): Exception(std::to_string(n.startToken.line) + ":" + std::to_string(n.startToken.column) + " Unresolved symbol " + name) {}
    };

    class TypeException: public Exception {
    public:
        TypeException(const std::string& msg, const AstNode& n): Exception(std::to_string(n.startToken.line) + ":" + std::to_string(n.startToken.column) + " " + msg) {}
    };
}

#endif