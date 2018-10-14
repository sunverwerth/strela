// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_ByteCodeChunk_h
#define Strela_VM_ByteCodeChunk_h

#include "Opcode.h"
#include "VMValue.h"
#include "VMType.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

#ifdef __APPLE__
    #include <ffi/ffi.h>
#else
    #include <ffi.h>
#endif


namespace Strela {
    class TypeDecl;
    class FuncDecl;
    class SourceFile;

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

    struct SourceLine {
        size_t address;
		size_t file;
		size_t line;
    };

    struct VarInfo {
        int offset;
        std::string name;
        VMType* type;
    };

    struct FunctionInfo {
        std::string name;
        std::vector<VarInfo> variables;
    };

    class ByteCodeChunk {
    public:
        std::vector<VMValue> constants;
        std::vector<Opcode> opcodes;
        std::map<size_t, FunctionInfo> functions;
        std::vector<ForeignFunction> foreignFunctions;
        std::vector<VMType*> types;
        size_t main;
        std::vector<const SourceFile*> files;
        std::vector<SourceLine> lines;

		const SourceLine* getLine(size_t address) const;
        void setLine(const SourceFile* file, size_t line);
        void addFunction(size_t address, const FunctionInfo& func);
        int addConstant(VMValue c);
        int addForeignFunction(FuncDecl& n);
        int addOp(Opcode code);
        int addOp(Opcode code, size_t argSize, const void* arg);
        int addOp(Opcode code, size_t argSize1, const void* arg1, size_t argSize2, const void* arg2);
        template <typename T> struct identity { using type = T; };
        template <typename T> int addOp(Opcode code, const typename identity<T>::type& arg) {
            return addOp(code, sizeof(T), &arg);
        }
        template <typename T, typename T2> int addOp(Opcode code, const typename identity<T>::type& arg, const typename identity<T2>::type& arg2) {
            return addOp(code, sizeof(T), &arg, sizeof(T2), &arg2);
        }
        void writeArgument(size_t pos, uint64_t arg);
        void write(size_t pos, void* data, size_t size);
    };

    std::ostream& operator<<(std::ostream& str, const ByteCodeChunk& chunk);
    std::istream& operator>>(std::istream& str, ByteCodeChunk& chunk);
}

#endif