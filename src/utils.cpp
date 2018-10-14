#include "utils.h"

namespace Strela {
    
    std::string unescape(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\\') {
                if (i < str.size() - 1) {
                    char c = str[++i];
                    switch (c) {
                        case '0': unescaped += '0'; break;
                        case 'r': unescaped += '\r'; break;
                        case 'n': unescaped += '\n'; break;
                        case 't': unescaped += '\t'; break;
                        case 'e': unescaped += '\x1b'; break;
                        case '\\': unescaped += '\\'; break;
                        case '"': unescaped += '"'; break;
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
                case '\0': escaped += "\\0"; break;
                case '\r': escaped += "\\r"; break;
                case '\n': escaped += "\\n"; break;
                case '\t': escaped += "\\t"; break;
                case '\x1b': escaped += "\\e"; break;
                case '\\': escaped += "\\\\"; break;
                case '"': escaped += "\\\""; break;
                default:
                escaped += str[i];
            }
        }
        return escaped;
    }
}