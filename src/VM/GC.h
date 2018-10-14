// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_GC_h
#define Strela_VM_GC_h

#include "VMValue.h"
#include "VMObject.h"
#include "VMType.h"

#include <vector>
#include <list>
#include <set>

namespace Strela {
    /**
     * Naive mark-and-sweep collector
     */
    class GC {
    public:
        void* allocObject(const VMType* type);
        void* allocArray(const VMType* type, uint64_t length);
        
        void collect(std::vector<VMValue>& stack);

        void lock(void* obj);
        void unlock(void* obj);
    
    private:
        void mark(VMObject* object);

    private:
        std::list<VMObject*> objects;
        std::set<VMObject*> lockList;
    };
}

#endif