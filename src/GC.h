// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_GC_h
#define Strela_GC_h

#include "VMValue.h"
#include "VMObject.h"

#include <vector>
#include <list>

namespace Strela {
    /**
     * Naive mark-and-sweep collector
     */
    class GC {
    public:
        VMObject* allocObject(size_t numFields);
        void collect(const std::vector<VMValue>& stack);
    
    private:
        void mark(VMObject* object);

    private:
        std::list<VMObject*> objects;
    };
}

#endif