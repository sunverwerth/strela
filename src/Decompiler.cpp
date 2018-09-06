// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "Decompiler.h"
#include "VM/ByteCodeChunk.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

namespace Strela {
    std::string escape(const std::string&);
    
    void Decompiler::printValue(const VMValue& c) const {
        std::cout << std::dec;
        if (c.type == VMValue::Type::integer) std::cout << "int(" << c.value.integer << ")";
        else if (c.type == VMValue::Type::floating) std::cout << "float(" << c.value.f64 << ")";
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
                std::cout << "\n; " << function->second.name << "\n";
            }
            size_t opStart = i;
            auto op = (Opcode)chunk.opcodes[i];
            auto info = opcodeInfo[(int)op];

            auto numArgs = info.argWidth;
            int width = numArgs + 1;
            std::vector<unsigned char> args;
            for (int a = 0; a < numArgs; ++a) {
                args.push_back((unsigned char)chunk.opcodes[++i]);
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
                int cpos = i - width - opcodeInfo[(int)Opcode::Const].argWidth;
                if (cpos >= 0 && (Opcode)chunk.opcodes[cpos] == Opcode::Const) {
                    auto constIndex = getArg(cpos);
                    auto address = chunk.constants[constIndex].value.integer;
                    auto it = chunk.functions.find(address);
                    if (it != chunk.functions.end()) {
                        std::cout << it->second.name;
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
			else if (op == Opcode::CallImm) {
                arg &= 0xffffffff;
				auto it = chunk.functions.find(arg);
				if (it != chunk.functions.end()) {
					std::cout << it->second.name;
				}
				else {
					int diff = (int)arg - (int)opStart;
					std::cout << std::dec << (diff > 0 ? "+" : "") << diff;
				}
				std::cout << " (0x" << std::setw(8) << std::setfill('0') << std::hex << std::right << arg << ")";
			}
			else if (op == Opcode::New) {
				if (arg < chunk.types.size()) {
					std::cout << chunk.types[arg]->name << " ";
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
                    else if (op == Opcode::F32) {
                        std::cout << *(float*)&arg;
                    }
                    else if (op == Opcode::F64) {
                        std::cout << *(double*)&arg;
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
        if (numargs > sizeof(arg)) numargs = sizeof(arg);
        memcpy(&arg, &chunk.opcodes[pos + 1], numargs);
        return arg;
    }
}