#ifndef Strela_utils_h
#define Strela_utils_h

#include <string>

namespace Strela {
    std::string escape(const std::string& str);
    std::string unescape(const std::string& str);
}

#endif