// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_ByteCodeChunk_h
#define Strela_ByteCodeChunk_h

#include "Opcode.h"
#include "VMValue.h"

#include <map>
#include <vector>
#include <iostream>
#include <ffi.h>

namespace Strela {
    class TypeDecl;
    class FuncDecl;

    class ForeignFunction {
    public:
        ForeignFunction(const std::string& name, TypeDecl* returnType, const std::vector<TypeDecl*>& argTypes): name(name), returnType(returnType), argTypes(argTypes) {}

        std::string name;
        TypeDecl* returnType;
        std::vector<TypeDecl*> argTypes;

        typedef void(*callback)(void);

        mutable ffi_type** ffi_argTypes = nullptr;
        mutable ffi_cif cif;
        mutable callback ptr = nullptr;
    };

    class ByteCodeChunk {
    public:
        std::vector<VMValue> constants;
        std::vector<char> opcodes;
        std::map<size_t, std::string> functions;
        std::vector<ForeignFunction> foreignFunctions;
        size_t main;

        void addFunction(size_t address, const std::string& name);
        int addConstant(VMValue c);
        int addForeignFunction(FuncDecl& n);
        int addOp(Opcode code);
        int addOp(Opcode code, size_t argSize, const void* arg);
        template <typename T> struct identity { using type = T; };
        template <typename T> int addOp(Opcode code, const typename identity<T>::type& arg) {
            return addOp(code, sizeof(T), &arg);
        }
        void writeArgument(size_t pos, uint64_t arg);
    };

    std::ostream& operator<<(std::ostream& str, const ByteCodeChunk& chunk);
    std::istream& operator>>(std::istream& str, ByteCodeChunk& chunk);
}

#endif