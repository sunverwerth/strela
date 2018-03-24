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

#include <cstring>
#include <chrono>
#include <cmath>

namespace Strela {

    extern int g_timeout;

    uint64_t millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    VM::VM(const ByteCodeChunk& chunk): chunk(chunk) {
		for (auto& ff: chunk.foreignFunctions) {
			double(*ptr)(double) = &sqrt;
			ff.ptr = reinterpret_cast<char*>(ptr);
		}

        frame = getFrame();
        frame->ip = chunk.main;
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
			op = chunk.opcodes[frame->ip++];
            if (op > numOpcodes) {
                throw std::runtime_error("Invalid opcode. (" + std::to_string(op) + ")");
            }

            auto& info = opcodeInfo[op];

            if (info.argWidth > 0) {
                memset(&arg, 0, sizeof(arg));
                memcpy(&arg.value, &chunk.opcodes[frame->ip], info.argWidth);
                arg.type = info.argType;
             
                frame->ip += info.argWidth;
            }

            switch ((Opcode)op) {
            case Opcode::INT:
            case Opcode::FLOAT:
                push(arg);
                break;

            case Opcode::Null:
                push(VMValue());
                break;

			case Opcode::Const: {
				push(chunk.constants[arg.value.integer]);
				break;
			}
			case Opcode::Param: {
				push(frame->arguments[arg.value.integer]);
				break;
			}
			case Opcode::StoreParam: {
				frame->arguments[arg.value.integer] = pop();
				break;
			}
			case Opcode::Var: {
                if (frame->variables.size() < arg.value.integer + 1) {
                    frame->variables.resize(arg.value.integer + 1);
                }
				push(frame->variables[arg.value.integer]);
				break;
			}
			case Opcode::StoreVar: {
                if (frame->variables.size() < arg.value.integer + 1) {
                    frame->variables.resize(arg.value.integer + 1);
                }
				frame->variables[arg.value.integer] = pop();
				break;
			}
			case Opcode::Peek: {
				push(frame->stack[frame->stack.size() - 1 - arg.value.integer]);
				break;
			}
			case Opcode::GrowStack: {
				frame->stack.resize(frame->stack.size() + arg.value.integer);
				break;
			}
			case Opcode::Call: {
				auto newip = pop().value.integer;
                auto newframe = getFrame();
                newframe->parent = frame;
                newframe->ip = newip;
                for (int i = 0; i < arg.value.integer; ++i) {
                    newframe->arguments.push_back(pop());
                }
                frame = newframe;
				break;
			}
			case Opcode::F2I: {
				push(VMValue((int64_t)pop().value.floating));
				break;
			}
			case Opcode::I2F: {
				push(VMValue((double)pop().value.integer));
				break;
			}
			case Opcode::NativeCall: {
				auto funcindex = pop();
				auto func = chunk.foreignFunctions[funcindex.value.integer].ptr;
				auto par = pop();
				typedef double(*mathfunc)(double);
				auto sqr = (mathfunc)func;
				push(VMValue(sqr(par.value.floating)));
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
                push(obj.value.object->getField(arg.value.integer));
                break;
            }
            case Opcode::FieldInd: {
                auto obj = pop();
                auto off = pop();
                push(obj.value.object->getField(off.value.integer + arg.value.integer));
                break;
            }
            case Opcode::StoreField: {
                auto obj = pop();
                auto val = pop();
                obj.value.object->setField(arg.value.integer, val);
                break;
            }
            case Opcode::StoreFieldInd: {
                auto obj = pop();
                auto off = pop();
                auto val = pop();
                obj.value.object->setField(off.value.integer + arg.value.integer, val);
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
				throw Exception(std::string("Opcode '") + info.name + "' not implemented");
			}
		}

        //uint64_t time = millis() - start + 1;
        //std::cerr << (opcounter/time) << " kOP/s\n";

        return VMValue();
    }
	
    size_t VM::push(VMValue val) {
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

            std::cout << name << "\n";

            cur = frame->parent;
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