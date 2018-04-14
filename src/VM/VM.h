// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_VM_h
#define Strela_VM_VM_h

#include "Opcode.h"
#include "VMValue.h"
#include "GC.h"

#include <vector>
#include <string>

namespace Strela {
    class ByteCodeChunk;
    class VMFrame;
    
    class VM {
    public:
        VM(const ByteCodeChunk& chunk, const std::vector<std::string>& arguments);
        VMValue run();

        void printCallStack();

    private:
        bool execute();

        size_t push(const VMValue& val);
        VMValue pop();
        void pop(size_t num);

        VMFrame* getFrame();
        void recycleFrame(VMFrame*);

        template<typename T> T read();

        std::vector<VMFrame*> framePool;

    public:
        const ByteCodeChunk& chunk;
        char op;
        GC gc;

        VMFrame* frame = nullptr;
    };
}

#endif