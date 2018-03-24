// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_h
#define Strela_VM_h

#include "Opcode.h"
#include "VMValue.h"
#include "GC.h"

#include <vector>

namespace Strela {
    class ByteCodeChunk;
    
    class VM {
    public:
        VM(const ByteCodeChunk& chunk);
        VMValue run();

        void printCallStack();

    private:
        bool execute();

        size_t push(VMValue val);
        VMValue pop();
        void pop(size_t num);

    public:
        const ByteCodeChunk& chunk;
        char op;
        VMValue arg;
        int64_t ip;
		int64_t sp;
        std::vector<VMValue> stack;
        GC gc;
    };
}

#endif