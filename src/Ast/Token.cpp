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