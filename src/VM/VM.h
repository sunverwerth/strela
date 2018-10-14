// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_VM_h
#define Strela_VM_VM_h

#include "Opcode.h"
#include "VMValue.h"
#include "GC.h"

#include <vector>
#include <string>
#include <map>

struct Frame {
    size_t bp;
    size_t ip;
};

namespace Strela {
    class ByteCodeChunk;

    class VM {
    public:
        VM(ByteCodeChunk& chunk, const std::vector<std::string>& arguments);
        VMValue run();
        void step(size_t maxOps);

        std::string printCallStack();

    private:
        void push(const VMValue& val);
        VMValue pop();
        void pop(size_t num);
		VMValue peek(size_t idx);
		void poke(size_t idx, const VMValue& val);

        template<typename T> T read();

		void checkRead(const VMValue& val, int64_t offset);
		void checkWrite(const VMValue& val, int64_t offset);

    public:
		enum {
			RUNNING,
			STOPPED,
			FINISHED,
		} status;

        bool halt = false;
        VMValue exitCode;
        int numallocs = 0;
        ByteCodeChunk& chunk;
        Opcode op;
        GC gc;
        size_t ip;
        size_t bp;
        std::vector<VMValue> stack;
        std::vector<Frame> callStack;
    };
}

#endif