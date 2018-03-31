// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "VM.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"
#include "exceptions.h"
#include "VMObject.h"
#include "Ast/InterfaceDecl.h"
#include "Ast/FuncDecl.h"
#include "VMFrame.h"
#include "Ast/FloatType.h"


#include <cstring>
#include <chrono>
#include <cmath>
#include <ffi.h>
#include <dlfcn.h>

namespace Strela {

    extern int g_timeout;

    uint64_t millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    VM::VM(const ByteCodeChunk& chunk, const std::vector<std::string>& arguments): chunk(chunk) {
		for (auto& ff: chunk.foreignFunctions) {
            ffi_type* rtype;
            rtype = &ffi_type_double;

            auto numArgs = ff.argTypes.size();
            ff.ffi_argTypes = new ffi_type*[numArgs];
            for (size_t i = 0; i < numArgs; ++i) {
                ff.ffi_argTypes[i] = &ffi_type_double;
            }
            ffi_prep_cif(&ff.cif, FFI_DEFAULT_ABI, numArgs, rtype, ff.ffi_argTypes);
            
			ff.ptr = ForeignFunction::callback(dlsym(RTLD_DEFAULT, ff.name.c_str()));
		}

        frame = getFrame();
        frame->ip = chunk.main;
		auto arr = gc.allocObject(arguments.size() + 1);
		arr->setField(0, VMValue((int64_t)arguments.size()));
		for (int i = 0; i < arguments.size(); ++i) {
			arr->setField(i + 1, VMValue(arguments[i].c_str()));
		}
		frame->arguments.push_back(VMValue(arr));
    }

    template<typename T> T VM::read() {
        T ret;
        memcpy(&ret, &chunk.opcodes[frame->ip], sizeof(T));
        frame->ip += sizeof(T);
        return ret;
    }

    VMValue VM::run() {
        uint64_t start = millis();
        int numallocs = 0;
        uint64_t opcounter = 0;
        bool halt = false;

		while (!halt) {
            if (g_timeout > 0 && (opcounter & 0x00ffffff) == 0 && millis() - start > g_timeout) {
                std::cerr << "Aborted due to timeout.\n";
                exit(1);
            }
            
            ++opcounter;
			
            op = read<char>();

            switch ((Opcode)op) {
            case Opcode::I8:
                push(VMValue((int64_t)read<int8_t>()));
                break;
            case Opcode::I16:
                push(VMValue((int64_t)read<int16_t>()));
                break;
            case Opcode::I32:
                push(VMValue((int64_t)read<int32_t>()));
                break;
            case Opcode::I64:
                push(VMValue((int64_t)read<int64_t>()));
                break;
            case Opcode::U8:
                push(VMValue((int64_t)read<uint8_t>()));
                break;
            case Opcode::U16:
                push(VMValue((int64_t)read<uint16_t>()));
                break;
            case Opcode::U32:
                push(VMValue((int64_t)read<uint32_t>()));
                break;
            case Opcode::U64:
                push(VMValue((int64_t)read<uint64_t>()));
                break;
            case Opcode::F32:
                push(VMValue((double)read<float>()));
                break;
            case Opcode::F64:
                push(VMValue((double)read<double>()));
                break;

            case Opcode::Null:
                push(VMValue());
                break;

			case Opcode::Const: {
				push(chunk.constants[read<uint8_t>()]);
				break;
			}
			case Opcode::Param: {
				push(frame->arguments[read<uint8_t>()]);
				break;
			}
			case Opcode::StoreParam: {
				frame->arguments[read<uint8_t>()] = pop();
				break;
			}
			case Opcode::Var: {
                auto arg = read<uint8_t>();
                if (frame->variables.size() < arg + 1) {
                    frame->variables.resize(arg + 1);
                }
				push(frame->variables[arg]);
				break;
			}
			case Opcode::StoreVar: {
                auto arg = read<uint8_t>();
                if (frame->variables.size() < arg + 1) {
                    frame->variables.resize(arg + 1);
                }
				frame->variables[arg] = pop();
				break;
			}
			case Opcode::Peek: {
				push(frame->stack[frame->stack.size() - 1 - read<uint8_t>()]);
				break;
			}
			case Opcode::Call: {
                auto numargs = read<uint8_t>();
				auto newip = pop().value.integer;
                auto newframe = getFrame();
                newframe->parent = frame;
                newframe->ip = newip;
                for (int i = 0; i < numargs; ++i) {
                    newframe->arguments.push_back(pop());
                }
                frame = newframe;
				break;
			}
			case Opcode::F64tI64: {
				push(VMValue((int64_t)pop().value.floating));
				break;
			}
			case Opcode::I64tF64: {
				push(VMValue((double)pop().value.integer));
				break;
			}
			case Opcode::NativeCall: {
				auto funcindex = pop();
				auto& ff = chunk.foreignFunctions[funcindex.value.integer];
				
                auto par = pop();

                VMValue retVal;
                std::vector<VMValue> args;
                std::vector<void*> argPtrs;

                args.push_back(par);
                argPtrs.push_back(&args[0].value.floating);

                ffi_call(&ff.cif, ff.ptr, &retVal.value.floating, &argPtrs[0]);

                retVal.type = VMValue::Type::floating;

				push(retVal);
				break;
			}
			case Opcode::Return: {
                auto oldframe = frame;
                frame = frame->parent;
				if (!frame) {
                    return oldframe->stack.back();
                }
                else {
                    push(oldframe->stack.back());
                }
                recycleFrame(oldframe);
				break;
			}
			case Opcode::ReturnVoid: {
                auto oldframe = frame;
                frame = frame->parent;
				if (!frame) {
                    return VMValue();
                }
                recycleFrame(oldframe);
				break;
			}
			case Opcode::Add: {
				auto r = pop();
				auto l = pop();
				push(l + r);
				break;
			}
			case Opcode::Sub: {
				auto r = pop();
				auto l = pop();
				push(l - r);
				break;
			}
			case Opcode::Mul: {
				auto r = pop();
				auto l = pop();
				push(l * r);
				break;
			}
			case Opcode::Div: {
				auto r = pop();
				auto l = pop();
				push(l / r);
				break;
			}
			case Opcode::CmpEQ: {
				auto r = pop();
				auto l = pop();
				push(l == r);
				break;
			}
			case Opcode::CmpNE: {
				auto r = pop();
				auto l = pop();
				push(l != r);
				break;
			}
			case Opcode::CmpLT: {
				auto r = pop();
				auto l = pop();
				push(l < r);
				break;
			}
			case Opcode::CmpGT: {
				auto r = pop();
				auto l = pop();
				push(l > r);
				break;
			}
			case Opcode::CmpLTE: {
				auto r = pop();
				auto l = pop();
				push(l <= r);
				break;
			}
			case Opcode::CmpGTE: {
				auto r = pop();
				auto l = pop();
				push(l >= r);
				break;
			}
			case Opcode::AndL: {
				auto r = pop();
				auto l = pop();
				push(l && r);
				break;
			}
			case Opcode::OrL: {
				auto r = pop();
				auto l = pop();
				push(l || r);
				break;
			}
			case Opcode::Not: {
				auto v = pop();
				push(!v);
				break;
			}
            case Opcode::Print: {
                auto val = pop();
                if (val.type == VMValue::Type::integer) std::cout << val.value.integer;
                else if (val.type == VMValue::Type::floating) std::cout << val.value.floating;
                else if (val.type == VMValue::Type::boolean) std::cout << (val.value.boolean ? "true" : "false");
                else if (val.type == VMValue::Type::null) std::cout << "null";
                else if (val.type == VMValue::Type::string) std::cout << val.value.string;
                else if (val.type == VMValue::Type::object) std::cout << "[object]";
                break;
            }
			case Opcode::Jmp: {
    			frame->ip = pop().value.integer;
				break;
			}
			case Opcode::JmpIf: {
				auto newip = pop();
				auto cond = pop();
				if (cond) {
					frame->ip = newip.value.integer;
				}
				break;
			}
			case Opcode::JmpIfNot: {
				auto newip = pop();
				auto cond = pop();
				if (!cond) {
					frame->ip = newip.value.integer;
				}
				break;
			}
            case Opcode::New: {
                numallocs++;
                if ((numallocs % 100) == 0) {
                    gc.collect(frame);
                }
                auto numfields = pop();
                auto obj = gc.allocObject(numfields.value.integer);
                push(VMValue(obj));
                break;
            }
            case Opcode::Field: {
                auto obj = pop();
                push(obj.value.object->getField(read<int8_t>()));
                break;
            }
            case Opcode::FieldInd: {
                auto obj = pop();
                auto off = pop();
                push(obj.value.object->getField(off.value.integer + read<int8_t>()));
                break;
            }
            case Opcode::StoreField: {
                auto obj = pop();
                auto val = pop();
                obj.value.object->setField(read<int8_t>(), val);
                break;
            }
            case Opcode::StoreFieldInd: {
                auto obj = pop();
                auto off = pop();
                auto val = pop();
                obj.value.object->setField(off.value.integer + read<int8_t>(), val);
                break;
            }
            case Opcode::Repeat: {
                push(frame->stack.back());
                break;
            }
            case Opcode::Pop: {
                pop();
                break;
            }
			case Opcode::Swap: {
				auto a = pop();
				auto b = pop();
				push(a);
				push(b);
				break;
			}
			default:
                if (op >= numOpcodes) {
                    throw Exception(std::string("Opcode '") + std::to_string(op) + "' not implemented");
                }
                else {
                    throw Exception(std::string("Opcode '") + opcodeInfo[op].name + "' not implemented");
                }
			}
		}

        return VMValue();
    }
	
    size_t VM::push(const VMValue& val) {
        frame->stack.push_back(val);
        return frame->stack.size() - 1;
    }

    VMValue VM::pop() {
        auto val = frame->stack.back();
        frame->stack.pop_back();
        return val;
    }

    void VM::pop(size_t num) {
        if (num == 0) return;
        frame->stack.resize(frame->stack.size() - num);
    }

    void VM::printCallStack() {
        auto cur = frame;
        while (cur) {
            std::string name("???");

            size_t largest = 0;
            for (auto&& it: chunk.functions) {
                if (it.first >= largest && it.first <= cur->ip) {
                    name = it.second;
                    largest = it.first;
                }
            }

            std::cout << name << " @" << cur->ip << "\n";

            cur = cur->parent;
        }
    }

    VMFrame* VM::getFrame() {
        if (framePool.empty()) {
            return new VMFrame();
        }
        else {
            auto fr = framePool.back();
            framePool.pop_back();
            fr->ip = 0;
            fr->parent = nullptr;
            fr->arguments.clear();
            fr->stack.clear();
            fr->variables.clear();
            return fr;
        }
    }

    void VM::recycleFrame(VMFrame* fr) {
        framePool.push_back(fr);
    }
}