#ifndef Strela_ByteCodeChunk_h
#define Strela_ByteCodeChunk_h

#include "Opcode.h"
#include "VMValue.h"

#include <map>
#include <vector>
#include <iostream>

namespace Strela {
    class ByteCodeChunk {
    public:
        std::vector<VMValue> constants;
        std::vector<char> opcodes;
        std::map<size_t, std::string> functions;
        size_t main;

        void addFunction(size_t address, const std::string& name);
        int addConstant(VMValue c);
        int addOp(Opcode code);
        int addOp(Opcode code, uint64_t arg);
        void writeArgument(size_t pos, uint64_t arg);
    };

    std::ostream& operator<<(std::ostream& str, const ByteCodeChunk& chunk);
    std::istream& operator>>(std::istream& str, ByteCodeChunk& chunk);
}

#endif