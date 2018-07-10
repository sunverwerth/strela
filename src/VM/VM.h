// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_VM_h
#define Strela_VM_VM_h

#include "Opcode.h"
#include "VMValue.h"
#include "GC.h"

#include <vector>
#include <string>

struct Frame {
    size_t bp;
    size_t ip;
};

namespace Strela {
    class ByteCodeChunk;
    
    class VM {
    public:
        VM(const ByteCodeChunk& chunk, const std::vector<std::string>& arguments);
        VMValue run();

        void printCallStack();

    private:
        bool execute();

        void push(const VMValue& val);
        VMValue pop();
        void pop(size_t num);
		VMValue peek(size_t idx);
		void poke(size_t idx, const VMValue& val);

        template<typename T> T read();

    public:
        const ByteCodeChunk& chunk;
        char op;
        GC gc;
        size_t ip;
        size_t bp;
        std::vector<VMValue> stack;
        std::vector<Frame> callStack;
    };
}

#endif