// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_GC_h
#define Strela_VM_GC_h

#include "VMValue.h"
#include "VMObject.h"
#include "VMType.h"

#include <vector>
#include <list>

namespace Strela {
    /**
     * Naive mark-and-sweep collector
     */
    class GC {
    public:
        VMObject* allocObject(const VMType* type);
        VMObject* allocArray(const VMType* type, uint64_t length);
        
        void collect(std::vector<VMValue>& stack);
    
    private:
        void mark(VMObject* object);

    private:
        std::list<VMObject*> objects;
    };
}

#endif