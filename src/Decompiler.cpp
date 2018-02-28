#include "Decompiler.h"
#include "ByteCodeChunk.h"
#include "Types/types.h"

#include <iostream>
#include <iomanip>
#include <cstring>

namespace Strela {
    std::string escape(const std::string&);
    
    void Decompiler::printValue(const VMValue& c) const {
        std::cout << std::dec;
        if (c.type == VMValue::Type::u8) std::cout << "u8(" << c.value.u8 << ")";
        else if (c.type == VMValue::Type::u16) std::cout << "u16(" << c.value.u16 << ")";
        else if (c.type == VMValue::Type::u32) std::cout << "u32(" << c.value.u32 << ")";
        else if (c.type == VMValue::Type::u64) std::cout << "u64(" << c.value.u64 << ")";
        else if (c.type == VMValue::Type::i8) std::cout << "i8(" << c.value.i8 << ")";
        else if (c.type == VMValue::Type::i16) std::cout << "i16(" << c.value.i16 << ")";
        else if (c.type == VMValue::Type::i32) std::cout << "i32(" << c.value.i32 << ")";
        else if (c.type == VMValue::Type::i64) std::cout << "i64(" << c.value.i64 << ")";
        else if (c.type == VMValue::Type::f32) std::cout << "f32(" << c.value.f32 << ")";
        else if (c.type == VMValue::Type::f64) std::cout << "f64(" << c.value.f64 << ")";
        else if (c.type == VMValue::Type::boolean) std::cout << (c.value.boolean ? "true" : "false");
        else if (c.type == VMValue::Type::string) std::cout << "\"" << escape(c.value.string) << "\"";
        else if (c.type == VMValue::Type::null) std::cout << "null";
        else if (c.type == VMValue::Type::object) std::cout << "(object)";
        else std::cout << "???";
    }

    void Decompiler::listing() const {
        std::cout << "; Constants\n";
        for (size_t i = 0; i < chunk.constants.size(); ++i) {
            auto c = chunk.constants[i];
            std::cout << std::dec << i << ": ";
            printValue(c);
            std::cout << "\n";
        }

        for (size_t i = 0; i < chunk.opcodes.size(); ++i) {
            auto function = chunk.functions.find(i);
            if (function != chunk.functions.end()) {
                std::cout << "\n; " << function->second << "\n";
            }
            size_t opStart = i;
            auto op = (Opcode)chunk.opcodes[i];
            auto info = opcodeInfo[(int)op];

            auto numArgs = info.argWidth;
            int width = numArgs + 1;
            std::vector<unsigned char> args;
            for (int a = 0; a < numArgs; ++a) {
                args.push_back(chunk.opcodes[++i]);
            }
            auto arg = getArg(opStart);

            std::cout << "0x" << std::right << std::setw(8) << std::setfill('0') << std::hex << opStart << " " << std::setw(2) << (int)op << " ";

            for (int a = 0; a < 4; ++a) {
                if (a < args.size()) {
                    std::cout << std::setw(2) << (int)args[a] << " ";
                }
                else {
                    std::cout << ".. ";
                }
            }
            
            std::cout << std::left << std::setfill(' ') << std::setw(16);
            
            if ((int)op > numOpcodes) {
                std::cout << "???";
            }
            else {
                std::cout << info.name;
            }

            if (op == Opcode::Call || op == Opcode::Jmp || op == Opcode::JmpIf || op == Opcode::JmpIfNot) {
                auto cpos = i - width - opcodeInfo[(int)Opcode::Const].argWidth;
                if (cpos >= 0 && (Opcode)chunk.opcodes[cpos] == Opcode::Const) {
                    auto constIndex = getArg(cpos);
                    auto address = chunk.constants[constIndex].value.u64;
                    auto it = chunk.functions.find(address);
                    if (it != chunk.functions.end()) {
                        std::cout << it->second;
                    }
                    else {
                        int diff = (int)address - (int)opStart;
                        std::cout << std::dec << (diff > 0 ? "+" : "") << diff;
                    }
                    std::cout << " (0x" << std::setw(8) << std::setfill('0') << std::hex << std::right << address << ")";
                }
                else {
                    std::cout << "(dynamic)";
                }
            }

            if (args.size()) {
                std::cout << std::dec;
                if (op == Opcode::Const) {
                    auto c = chunk.constants[arg];
                    printValue(c);
                }
                else {
                    if (op == Opcode::ReturnVoid || op == Opcode::Return) {
                        if (arg > 0) {
                            std::cout << "pop "<< (int)arg;
                        }
                    }
                    else {
                        std::cout << (int)arg;
                    }
                }
            }
            std::cout << "\n";
        }
    }

    uint64_t Decompiler::getArg(size_t pos) const {
        auto op = chunk.opcodes[pos];
        auto numargs = opcodeInfo[(int)op].argWidth;
        uint64_t arg = 0;
        memcpy(&arg, &chunk.opcodes[pos + 1], numargs);
        return arg;
    }
}