#include "VM.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"
#include "exceptions.h"
#include "Types/types.h"
#include "VMObject.h"

#include <cstring>
#include <chrono>

namespace Strela {

    extern int g_timeout;

    uint64_t millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    VM::VM(const ByteCodeChunk& chunk): chunk(chunk), ip(chunk.main), sp(0) {
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
			op = chunk.opcodes[ip++];
            if (op > numOpcodes) {
                throw std::runtime_error("Invalid opcode. (" + std::to_string(op) + ")");
            }

            auto& info = opcodeInfo[op];

            if (info.argWidth > 0) {
                memset(&arg, 0, sizeof(arg));
                memcpy(&arg.value, &chunk.opcodes[ip], info.argWidth);
                arg.type = info.argType;
             
                ip += info.argWidth;
            }

            switch ((Opcode)op) {
            case Opcode::U8:
            case Opcode::U16:
            case Opcode::U32:
            case Opcode::U64:
            case Opcode::I8:
            case Opcode::I16:
            case Opcode::I32:
            case Opcode::I64:
            case Opcode::F32:
            case Opcode::F64:
                push(arg);
                break;

            case Opcode::Null:
                push(VMValue());
                break;

			case Opcode::Const: {
				push(chunk.constants[arg.value.u8]);
				break;
			}
			case Opcode::Param: {
				push(stack[sp - arg.value.u8 - 2]);
				break;
			}
			case Opcode::StoreParam: {
				stack[sp - arg.value.u8 - 2] = pop();
				break;
			}
			case Opcode::Var: {
				push(stack[sp + arg.value.u8]);
				break;
			}
			case Opcode::StoreVar: {
				stack[sp + arg.value.u8] = pop();
				break;
			}
			case Opcode::GrowStack: {
				stack.resize(stack.size() + arg.value.u8);
				break;
			}
			case Opcode::Call: {
				auto newip = pop().value.u64;
				push(VMValue(uint64_t(ip)));
				sp = push(VMValue(uint64_t(sp)));
				ip = newip;
				break;
			}
			case Opcode::Return: {
				if (sp == 0) {
                    halt = true;
                }
                else {
                    auto val = pop();

                    stack.resize(sp + 1);
                    sp = pop().value.u64;
                    ip = pop().value.u64;
                    pop(arg.value.u8);
                    push(val);
                }
				break;
			}
			case Opcode::ReturnVoid: {
				if (sp == 0) {
                    halt = true;
                }
                else {
                    stack.resize(sp + 1);
                    sp = pop().value.u64;
                    ip = pop().value.u64;
                    pop(arg.value.u8);
                }
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
                if (val.type == VMValue::Type::u8) std::cout << (int)val.value.u8;
                else if (val.type == VMValue::Type::u16) std::cout << val.value.u16;
                else if (val.type == VMValue::Type::u32) std::cout << val.value.u32;
                else if (val.type == VMValue::Type::u64) std::cout << val.value.u64;
                else if (val.type == VMValue::Type::i8) std::cout << (int)val.value.i8;
                else if (val.type == VMValue::Type::i16) std::cout << val.value.i16;
                else if (val.type == VMValue::Type::i32) std::cout << val.value.i32;
                else if (val.type == VMValue::Type::i64) std::cout << val.value.i64;
                else if (val.type == VMValue::Type::f32) std::cout << val.value.f32;
                else if (val.type == VMValue::Type::f64) std::cout << val.value.f64;
                else if (val.type == VMValue::Type::boolean) std::cout << (val.value.boolean ? "true" : "false");
                else if (val.type == VMValue::Type::null) std::cout << "null";
                else if (val.type == VMValue::Type::string) std::cout << val.value.string;
                else if (val.type == VMValue::Type::object) std::cout << "(object)";
                break;
            }
			case Opcode::Jmp: {
    			ip = pop().value.u64;
				break;
			}
			case Opcode::JmpIf: {
				auto newip = pop();
				auto cond = pop();
				if (cond) {
					ip = newip.value.u64;
				}
				break;
			}
			case Opcode::JmpIfNot: {
				auto newip = pop();
				auto cond = pop();
				if (!cond) {
					ip = newip.value.u64;
				}
				break;
			}
            case Opcode::New: {
                numallocs++;
                if ((numallocs % 100) == 0) {
                    gc.collect(stack);
                }
                auto obj = gc.allocObject(arg.value.u8);
                push(VMValue(obj));
                break;
            }
            case Opcode::Field: {
                auto obj = pop();
                push(obj.value.object->getField(arg.value.u8));
                break;
            }
            case Opcode::StoreField: {
                auto obj = pop();
                obj.value.object->setField(arg.value.u8, pop());
                break;
            }
            case Opcode::Repeat: {
                push(stack.back());
                break;
            }
            case Opcode::Pop: {
                pop();
                break;
            }
			default:
				throw Exception(std::string("Opcode '") + info.name + "' not implemented");
			}
		}

        //uint64_t time = millis() - start + 1;
        //std::cerr << (opcounter/time) << " kOP/s\n";

        return stack.size() ? stack.back() : VMValue();
    }
	
    size_t VM::push(VMValue val) {
        stack.push_back(val);
        return stack.size() - 1;
    }

    VMValue VM::pop() {
        auto val = stack.back();
        stack.pop_back();
        return val;
    }

    void VM::pop(size_t num) {
        if (num == 0) return;
        stack.resize(stack.size() - num);
    }

    void VM::printCallStack() {
        auto cur = sp;
        auto fun = ip;
        while (cur > 0) {
            std::string name("???");

            size_t largest = 0;
            for (auto&& it: chunk.functions) {
                if (it.first >= largest && it.first <= fun) {
                    name = it.second;
                    largest = it.first;
                }
            }

            std::cout << name << "\n";

            cur = stack[cur].value.u64;
            fun = stack[cur - 1].value.u64;
        }
    }
}