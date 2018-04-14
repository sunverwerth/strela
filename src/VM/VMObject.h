// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_VMObject_h
#define Strela_VM_VMObject_h

#include "VMValue.h"

#include <vector>

namespace Strela {
    class VMType;

    struct VMObject {
        bool marked = false;
        const VMType* type;
        char data[];
    };
}

#endif