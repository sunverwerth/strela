// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "VM.h"
#include "VMObject.h"
#include "VMFrame.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"

#include "../exceptions.h"
#include "../Ast/InterfaceDecl.h"
#include "../Ast/FuncDecl.h"
#include "../Ast/FloatType.h"
#include "../Ast/IntType.h"
#include "../Ast/PointerType.h"
#include "../Ast/VoidType.h"


#include <cstring>
#include <chrono>
#include <cmath>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

#ifdef __APPLE__
    #include <ffi/ffi.h>
#else
    #include <ffi.h>
#endif

namespace Strela {

    extern int g_timeout;

    uint64_t millis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    ffi_type* ffitype(const TypeDecl* t) {
        if (t == &IntType::u8)
            return &ffi_type_uint8;
        if (t == &IntType::u16)
            return &ffi_type_uint16;
        if (t == &IntType::u32)
            return &ffi_type_uint32;
        if (t == &IntType::u64)
            return &ffi_type_uint64;
        if (t == &IntType::i8)
            return &ffi_type_sint8;
        if (t == &IntType::i16)
            return &ffi_type_sint16;
        if (t == &IntType::i32)
            return &ffi_type_sint32;
        if (t == &IntType::i64)
            return &ffi_type_sint64;
        if (t == &FloatType::f32)
            return &ffi_type_float;
        if (t == &FloatType::f64)
            return &ffi_type_double;

        return &ffi_type_pointer;
    }

    VM::VM(const ByteCodeChunk& chunk, const std::vector<std::string>& arguments): chunk(chunk) {
#ifdef _WIN32
		auto mod = LoadLibrary("msvcrt.dll");
#endif

		for (auto& ff: chunk.foreignFunctions) {
            ffi_type* rtype;
            rtype = ffitype(ff.returnType);

            auto numArgs = ff.argTypes.size();
            ff.ffi_argTypes = new ffi_type*[numArgs];
            for (size_t i = 0; i < numArgs; ++i) {
                ff.ffi_argTypes[i] = ffitype(ff.argTypes[i]);
            }
            ffi_prep_cif(&ff.cif, FFI_DEFAULT_ABI, numArgs, rtype, ff.ffi_argTypes);

#ifdef _WIN32
			ff.ptr = ForeignFunction::callback(GetProcAddress(mod, ff.name.c_str()));
#else
			ff.ptr = ForeignFunction::callback(dlsym(RTLD_DEFAULT, ff.name.c_str()));
#endif
			
			if (!ff.ptr) {
				std::cerr << "Unresolved external symbol \"" << ff.name << "\"\n";
				exit(1);
			}
		}

        frame = getFrame();
        frame->ip = chunk.main;
        VMType* arrtype = nullptr;
        for (auto&& type: chunk.types) {
            if (type->name == "String[]") {
                arrtype = type;
                break;
            }
        }
		auto arr = gc.allocArray(arrtype, arguments.size());
		for (int i = 0; i < arguments.size(); ++i) {
			//arr->setElement(i, VMValue(arguments[i].c_str()));
		}
		frame->stack.push_back(VMValue(arr));
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
            case Opcode::F32: {
                auto f = read<float>();
                VMValue val;
                memcpy(&val, &f, sizeof(float));
                val.type = VMValue::Type::floating;
                push(val);
                break;
            }
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
            case Opcode::Grow: {
                frame->stack.resize(frame->stack.size() + read<uint8_t>());
                break;
            }
			case Opcode::Var: {
				push(frame->stack[read<uint8_t>()]);
				break;
			}
			case Opcode::StoreVar: {
				frame->stack[read<uint8_t>()] = pop();
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
                    newframe->stack.push_back(pop());
                }
                frame = newframe;
				break;
			}
			case Opcode::F32tI64: {
                float f;
                auto val = pop();
                memcpy(&f, &val, sizeof(float));
				push(VMValue((int64_t)f));
				break;
			}
			case Opcode::F64tI64: {
				push(VMValue((int64_t)pop().value.floating));
				break;
			}
			case Opcode::I64tF32: {
                auto val = pop();
                float f = (float)val.value.integer;
                val.value.floating = 0;
                memcpy(&val, &f, sizeof(float));
				push(val);
				break;
			}
			case Opcode::I64tF64: {
				push(VMValue((double)pop().value.integer));
				break;
			}
			case Opcode::F64tF32: {
                auto val = pop();
                float f = (float)val.value.floating;
                val.value.floating = 0;
                memcpy(&val, &f, sizeof(float));
				push(val);
				break;
			}
			case Opcode::F32tF64: {
                auto val = pop();
                float f;
                memcpy(&f, &val, sizeof(float));
                val.value.floating = f;
				push(val);
				break;
			}
			case Opcode::NativeCall: {
				auto funcindex = pop();
				auto& ff = chunk.foreignFunctions[funcindex.value.integer];
				
                VMValue retVal((int64_t)0);
                if (ff.returnType->as<FloatType>()) retVal.type = VMValue::Type::floating;
                else if (ff.returnType->as<IntType>()) retVal.type = VMValue::Type::integer;

                std::vector<VMValue> originalArgs;
                originalArgs.reserve(ff.argTypes.size());
                for (size_t i = 0; i < ff.argTypes.size(); ++i) {
                    originalArgs.push_back(pop());
                }
				
                std::vector<VMValue> args;
                args.reserve(ff.argTypes.size());
                std::vector<void*> argPtrs;
                argPtrs.reserve(ff.argTypes.size());
                for (size_t i = 0; i < ff.argTypes.size(); ++i) {
                    if (originalArgs[i].type == VMValue::Type::object) {
                        void* aptr = &originalArgs[i].value.object->data[0];
						VMValue arg((int64_t)0);
                        memcpy(&arg.value.integer, &aptr, sizeof(void*));
                        args.push_back(arg);
                        argPtrs.push_back(&args.back());
                    }
                    else if (ff.argTypes[i] == &PointerType::instance) {
                        void* aptr = &originalArgs[i].value;
                        VMValue arg((int64_t)0);
                        memcpy(&arg.value.integer, &aptr, sizeof(void*));
                        args.push_back(arg);
                        argPtrs.push_back(&args.back());
                    }
                    else {
                        argPtrs.push_back(&originalArgs[i]);
                    }
                }

                ffi_call(&ff.cif, ff.ptr, &retVal, ff.argTypes.empty() ? nullptr : &argPtrs[0]);
                if (errno > 0) {
                    auto err = strerror(errno);
                    std::cerr << ff.name << ": " << err << "\n";
                    errno = 0;
                }

                if (ff.returnType != &VoidType::instance) {
				    push(retVal);
                }
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
			case Opcode::AddI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(int64_t(l.value.integer + r.value.integer)));
				break;
			}
			case Opcode::AddF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(double(a + b)));
				break;
			}
			case Opcode::AddF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(double(a + b)));
				break;
			}
			case Opcode::SubI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(int64_t(l.value.integer - r.value.integer)));
				break;
			}
            case Opcode::SubF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(double(a - b)));
				break;
			}
			case Opcode::SubF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(double(a - b)));
				break;
			}
			case Opcode::MulI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(int64_t(l.value.integer * r.value.integer)));
				break;
			}
			case Opcode::MulF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(double(a * b)));
				break;
			}
			case Opcode::MulF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(double(a * b)));
				break;
			}
			case Opcode::DivI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(int64_t(l.value.integer / r.value.integer)));
				break;
			}
			case Opcode::DivF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(double(a / b)));
				break;
			}
			case Opcode::DivF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(double(a / b)));
				break;
			}
			case Opcode::CmpEQ: {
				auto r = pop();
				auto l = pop();
				push(l == r);
				break;
			}
			case Opcode::CmpEQS: {
				auto r = pop();
				auto l = pop();
				push(VMValue(bool(strcmp(r.value.string, l.value.string) == 0)));
				break;
			}
			case Opcode::CmpNE: {
				auto r = pop();
				auto l = pop();
				push(l != r);
				break;
			}
            case Opcode::CmpLTI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(bool(l.value.integer < r.value.integer)));
				break;
			}
			case Opcode::CmpLTF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(bool(a < b)));
				break;
			}
			case Opcode::CmpLTF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(bool(a < b)));
				break;
			}
			case Opcode::CmpGTI: {
				auto r = pop();
				auto l = pop();
				push(VMValue(bool(l.value.integer > r.value.integer)));
				break;
			}
			case Opcode::CmpGTF32: {
				auto r = pop();
				auto l = pop();
                float a, b;
                memcpy(&a, &l, sizeof(float));
                memcpy(&b, &r, sizeof(float));
				push(VMValue(bool(a > b)));
				break;
			}
			case Opcode::CmpGTF64: {
				auto r = pop();
				auto l = pop();
                double a, b;
                memcpy(&a, &l, sizeof(double));
                memcpy(&b, &r, sizeof(double));
				push(VMValue(bool(a > b)));
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
				push(VMValue(l.value.boolean && r.value.boolean));
				break;
			}
			case Opcode::OrL: {
				auto r = pop();
				auto l = pop();
				push(VMValue(l.value.boolean || r.value.boolean));
				break;
			}
			case Opcode::Not: {
				auto v = pop();

				push(VMValue(!v.value.boolean));
				break;
			}
            case Opcode::PrintI: {
                std::cout << pop().value.integer;
                break;
            }
            case Opcode::PrintF32: {
                float f;
                auto v = pop();
                memcpy(&f, &v, sizeof(float));
                std::cout << f;
                break;
            }
            case Opcode::PrintF64: {
                std::cout << pop().value.floating;
                break;
            }
            case Opcode::PrintN: {
                std::cout << "(null)";
                break;
            }
            case Opcode::PrintS: {
                auto val = pop();
                std::cout << val.value.string;
                break;
            }
            case Opcode::PrintO: {
                std::cout << "[object]";
                break;
            }
            case Opcode::PrintB: {
                std::cout << (pop().value.boolean ? "true" : "false");
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
                if ((numallocs % 1000) == 0) {
                    gc.collect(frame);
                }
                auto type = pop();
                auto obj = gc.allocObject(chunk.types[type.value.integer]);
                push(VMValue(obj));
                break;
            }
            case Opcode::Array: {
                numallocs++;
                if ((numallocs % 1000) == 0) {
                    gc.collect(frame);
                }
                auto length = pop();
                auto type = pop();
                auto obj = gc.allocArray(chunk.types[type.value.integer], length.value.integer);
                push(VMValue(obj));
                break;
            }
            case Opcode::Field8: {
            case Opcode::Field16:
            case Opcode::Field32:
            case Opcode::Field64: 
                size_t size;
                switch ((Opcode)op) {
                    case Opcode::Field8: size = 1; break;
                    case Opcode::Field16: size = 2; break;
                    case Opcode::Field32: size = 4; break;
                    case Opcode::Field64: size = 8; break;
                    default: exit(1);
                }
                auto obj = pop();
                auto offset = read<int8_t>();
                VMValue val((int64_t)0);
                memcpy(&val.value.integer, &obj.value.object->data[offset], size);
                val.type = VMValue::Type::integer;
                push(val);
                break;
            }
            case Opcode::FieldInd8: {
            case Opcode::FieldInd16:
            case Opcode::FieldInd32:
            case Opcode::FieldInd64:
                size_t size;
                switch ((Opcode)op) {
                    case Opcode::FieldInd8: size = 1; break;
                    case Opcode::FieldInd16: size = 2; break;
                    case Opcode::FieldInd32: size = 4; break;
                    case Opcode::FieldInd64: size = 8; break;
                    default: exit(1);
                }
                auto obj = pop();
                auto off = pop();
                auto off2 = read<int8_t>();
                VMValue val((int64_t)0);
                memcpy(&val.value.integer, &obj.value.object->data[off.value.integer + off2], size);
                val.type = VMValue::Type::integer;
                push(val);
                break;
            }
            case Opcode::StoreField8: {
            case Opcode::StoreField16:
            case Opcode::StoreField32:
            case Opcode::StoreField64: 
                size_t size;
                switch ((Opcode)op) {
                    case Opcode::StoreField8: size = 1; break;
                    case Opcode::StoreField16: size = 2; break;
                    case Opcode::StoreField32: size = 4; break;
                    case Opcode::StoreField64: size = 8; break;
                    default: exit(1);
                }
                auto obj = pop();
                auto val = pop();
                auto offset = read<int8_t>();
                memcpy(&obj.value.object->data[offset], &val.value.integer, size);
                break;
            }
            case Opcode::StoreFieldInd8: {
            case Opcode::StoreFieldInd16:
            case Opcode::StoreFieldInd32:
            case Opcode::StoreFieldInd64:
                size_t size;
                switch ((Opcode)op) {
                    case Opcode::StoreFieldInd8: size = 1; break;
                    case Opcode::StoreFieldInd16: size = 2; break;
                    case Opcode::StoreFieldInd32: size = 4; break;
                    case Opcode::StoreFieldInd64: size = 8; break;
                    default: exit(1);
                }
                auto obj = pop();
                auto off = pop();
                auto val = pop();
                auto off2 = read<int8_t>();
                memcpy(&obj.value.object->data[off.value.integer + off2], &val.value.integer, size);
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
            case Opcode::ConcatSS: {
                auto r = pop();
                auto l = pop();
                r.type = l.type = VMValue::Type::string;
                push(l + r);
                break;
            }
            case Opcode::ConcatSI: {
                auto r = pop();
                auto l = pop();
                l.type = VMValue::Type::string;
                r.type = VMValue::Type::integer;
                push(l + r);
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
            fr->stack.clear();
            return fr;
        }
    }

    void VM::recycleFrame(VMFrame* fr) {
        framePool.push_back(fr);
    }
}