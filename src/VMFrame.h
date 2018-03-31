#ifndef Strela_VMFrame_h
#define Strela_VMFrame_h

#include "VMValue.h"

#include <vector>

namespace Strela {
    class VMFrame {
    public:
        VMFrame* parent = nullptr;
        std::vector<VMValue> stack;
        size_t ip;
    };
}
#endif