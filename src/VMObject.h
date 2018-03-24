// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VMObject_h
#define Strela_VMObject_h

#include "VMValue.h"

#include <vector>

namespace Strela {
    class VMObject {
    public:
        VMObject(size_t numfields);
        VMValue getField(size_t index);
        void setField(size_t index, const VMValue& val);

    public:
        int flags = 0;
        std::vector<VMValue> fields;
    };
}

#endif