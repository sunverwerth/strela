// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "ByteCodeChunk.h"
#include "../exceptions.h"
#include "../Ast/FuncDecl.h"
#include "../Ast/FuncType.h"

#include <string.h>

namespace Strela {
    int ByteCodeChunk::addConstant(VMValue c) {
        for (int i = 0; i < constants.size(); ++i) {
            if (constants[i].equals(c)) {
                return i;
            }
        }
        constants.push_back(c);
        return constants.size() - 1;
    }

    int ByteCodeChunk::addForeignFunction(FuncDecl& n) {
        for (int i = 0; i < foreignFunctions.size(); ++i) {
            if (
                foreignFunctions[i].name == n.name &&
                foreignFunctions[i].returnType == n.type->returnType &&
                foreignFunctions[i].argTypes == n.type->paramTypes
            ) {
                return i;
            }
        }
        foreignFunctions.push_back(ForeignFunction(n.name, n.type->returnType, n.type->paramTypes));
        return foreignFunctions.size() - 1;
    }

    int ByteCodeChunk::addOp(Opcode code) {
        if (opcodeInfo[(int)code].argWidth > 0) throw Exception(std::string("Opcode ") + opcodeInfo[(int)code].name + " requires arguments.");
        opcodes.push_back((char)code);
        return opcodes.size() - 1;
    }

    int ByteCodeChunk::addOp(Opcode code, size_t argSize, const void* arg) {
        auto opwidth = opcodeInfo[(int)code].argWidth;
        if (opwidth != argSize) throw Exception(std::string("Opcode ") + opcodeInfo[(int)code].name + " has mismatching argument size. "
        + std::to_string(opwidth) + " expected but got " + std::to_string(argSize)
        );
        auto opAddr = opcodes.size();
        opcodes.push_back((char)code);
        opcodes.resize(opcodes.size() + argSize);
        memcpy(&opcodes[opAddr + 1], arg, argSize);
        return opAddr;
    }

    int ByteCodeChunk::addOp(Opcode code, size_t argSize1, const void* arg1, size_t argSize2, const void* arg2) {
        auto opwidth = opcodeInfo[(int)code].argWidth;
        if (opwidth != argSize1 + argSize2) throw Exception(std::string("Opcode ") + opcodeInfo[(int)code].name + " has mismatching argument size. "
        + std::to_string(opwidth) + " expected but got " + std::to_string(argSize1 + argSize2)
        );
        auto opAddr = opcodes.size();
        opcodes.push_back((char)code);
        opcodes.resize(opcodes.size() + argSize1 + argSize2);
        memcpy(&opcodes[opAddr + 1], arg1, argSize1);
        memcpy(&opcodes[opAddr + 1 + argSize1], arg2, argSize2);
        return opAddr;
    }

    void ByteCodeChunk::addFunction(size_t address, const std::string& name) {
        functions.insert(std::make_pair(address, name));
    }

    void ByteCodeChunk::writeArgument(size_t pos, uint64_t arg) {
        auto numargs = opcodeInfo[(int)opcodes[pos++]].argWidth;
        if (pos + numargs > opcodes.size()) {
            opcodes.resize(pos + numargs);
        }
        memcpy(&opcodes[pos], &arg, numargs);
    }

    void ByteCodeChunk::write(size_t pos, void* data, size_t size) {
        memcpy(&opcodes[pos], data, size);
    }

    std::ostream& operator<<(std::ostream& str, const ByteCodeChunk& chunk) {
        str.write("STBC", 4);

        uint32_t numFunctions = chunk.functions.size();
        str.write((const char*)&numFunctions, 4);
        for (auto&& function: chunk.functions) {
            uint64_t offset = function.first;
            uint64_t len = function.second.size();
            const char* name = function.second.c_str();
            str.write((const char*)&offset, 8);
            str.write((const char*)&len, 8);
            str.write((const char*)name, len);
        }

        uint32_t numConstants = chunk.constants.size();
        str.write((const char*)&numConstants, 4);
        for (auto&& constant: chunk.constants) {
            str << constant;
        }
        
        uint64_t main = chunk.main;
        str.write((const char*)&main, 8);
        
        uint64_t numOpcodes = chunk.opcodes.size();
        str.write((const char*)&numOpcodes, 8);
        str.write((const char*)chunk.opcodes.data(), sizeof(char) * numOpcodes);
        
        return str;
    }

    std::istream& operator>>(std::istream& str, ByteCodeChunk& chunk) {
        chunk.constants.clear();
        chunk.functions.clear();
        chunk.opcodes.clear();

        char magic[5]{0};
        str.read((char*)&magic, 4);
        if (strcmp(magic, "STBC")) {
            throw std::runtime_error("Invalid bytecode format");
        }

        uint32_t numFunctions;
        str.read((char*)&numFunctions, 4);
        for (size_t i = 0; i < numFunctions; ++i) {
            uint64_t offset;
            str.read((char*)&offset, 8);
            uint64_t len;
            str.read((char*)&len, 8);
            char* name = new char[len + 1];
            name[len] = 0;
            str.read((char*)name, len);
            chunk.functions.insert(std::make_pair(offset, std::string(name)));
            delete[] name;
        }

        uint32_t numConstants;
        str.read((char*)&numConstants, 4);
        for (size_t i = 0; i < numConstants; ++i) {
            VMValue constant;
            str >> constant;
            chunk.constants.push_back(constant);
        }
        
        uint64_t main;
        str.read((char*)&main, 8);
        chunk.main = main;
        
        uint64_t numOpcodes;
        str.read((char*)&numOpcodes, 8);
        for (size_t i = 0; i < numOpcodes; ++i) {
            char op;
            str.read((char*)&op, 1);
            chunk.opcodes.push_back(op);
        }
        
        return str;
    }

}