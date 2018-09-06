// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "VM.h"
#include "VMObject.h"
#include "ByteCodeChunk.h"
#include "Opcode.h"

#include "../exceptions.h"
#include "../Ast/InterfaceDecl.h"
#include "../Ast/FuncDecl.h"
#include "../Ast/FloatType.h"
#include "../Ast/IntType.h"
#include "../Ast/PointerType.h"
#include "../Ast/VoidType.h"

#include <sstream>
#include <cstring>
#include <chrono>
#include <cmath>
#include <iomanip>

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

    uint64_t micros() {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    uint64_t nanos() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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

    VM::VM(ByteCodeChunk& chunk, const std::vector<std::string>& arguments): chunk(chunk), status(RUNNING) {
#ifdef _WIN32
		auto mod = LoadLibrary("msvcrt.dll");
		auto sockmod = LoadLibrary("ws2_32.dll");
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
			if (!ff.ptr) {
				ff.ptr = ForeignFunction::callback(GetProcAddress(sockmod, ff.name.c_str()));
			}
#else
			ff.ptr = ForeignFunction::callback(dlsym(RTLD_DEFAULT, ff.name.c_str()));
#endif
			
			if (!ff.ptr) {
				std::cerr << "Unresolved external symbol \"" << ff.name << "\"\n";
				exit(1);
			}
		}

        ip = chunk.main;
        bp = 0;
        VMType* arrtype = nullptr;
        for (auto&& type: chunk.types) {
            if (type->name == "String[]") {
                arrtype = type;
                break;
            }
        }
		auto arr = gc.allocArray(arrtype, arguments.size());
		auto data = (char*)arr;
		data += 8;
		for (int i = 0; i < arguments.size(); ++i) {
			*(uint64_t*)data = (uint64_t)arguments[i].c_str();
			//*(const char**)((char*)arr + sizeof(char*) * i) = arguments[i].c_str();
			data += 8;
		}
		push(VMValue(arr));
    }

    template<typename T> T VM::read() {
        T ret;
        memcpy(&ret, &chunk.opcodes[ip], sizeof(T));
        ip += sizeof(T);
        return ret;
    }

    VMValue VM::run() {
        uint64_t start = millis();

		while (status != FINISHED) {
            step(0xffffff);

			if (g_timeout > 0 && millis() - start > g_timeout) {
				std::cerr << "Aborted due to timeout.\n";
				return VMValue((int64_t)-1);
			}
		}

        return exitCode;
    }

    void VM::step(size_t maxOps) {
		while (maxOps--) {
			op = read<Opcode>();

			switch (op) {
			case Opcode::Trap: {
				status = STOPPED;
				ip--;
				return;
				break;
			}
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
				VMValue val;
				val.value.f32 = read<float>();
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
				push(chunk.constants[read<uint16_t>()]);
				break;
			}
			case Opcode::Grow: {
				stack.resize(stack.size() + read<uint8_t>());
				break;
			}
			case Opcode::Var: {
				push(peek(bp + read<uint8_t>()));
				break;
			}
			case Opcode::StoreVar: {
				poke(bp + read<uint8_t>(), pop());
				break;
			}
			case Opcode::Peek: {
				push(peek(stack.size() - 1 - read<uint8_t>()));
				break;
			}
			case Opcode::Call: {
				auto newip = pop().value.integer;
				auto numargs = read<uint8_t>();
				callStack.push_back({ bp, ip - 1 });
				bp = stack.size() - numargs;
				ip = newip;
				break;
			}
			case Opcode::CallImm: {
				auto newip = read<uint32_t>();
				auto numargs = read<uint8_t>();
				callStack.push_back({ bp, ip - 1 });
				bp = stack.size() - numargs;
				ip = newip;
				break;
			}
			case Opcode::F32tI64: {
				float f;
				auto val = pop();
				val.value.f64 = val.value.f32;
				push(val);
				break;
			}
			case Opcode::F64tI64: {
				push(VMValue((int64_t)pop().value.f64));
				break;
			}
			case Opcode::I64tF32: {
				auto val = pop();
				val.type = VMValue::Type::floating;
				val.value.f32 = val.value.integer;
				push(val);
				break;
			}
			case Opcode::I64tF64: {
				push(VMValue((double)pop().value.integer));
				break;
			}
			case Opcode::F64tF32: {
				auto val = pop();
				float f = (float)val.value.f64;
				val.value.f64 = 0;
				memcpy(&val, &f, sizeof(float));
				push(val);
				break;
			}
			case Opcode::F32tF64: {
				auto val = pop();
				float f;
				memcpy(&f, &val, sizeof(float));
				val.value.f64 = f;
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
				originalArgs.resize(ff.argTypes.size());
				for (int i = ff.argTypes.size() - 1; i >= 0; --i) {
					originalArgs[i] = pop();
				}

				std::vector<VMValue> args;
				args.reserve(ff.argTypes.size());
				std::vector<void*> argPtrs;
				argPtrs.reserve(ff.argTypes.size());
				for (size_t i = 0; i < ff.argTypes.size(); ++i) {
					if (originalArgs[i].type == VMValue::Type::object) {
						void* aptr = originalArgs[i].value.object;
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
				auto retVal = pop();
				if (callStack.empty()) {
					exitCode = retVal;
					status = FINISHED;
					maxOps = 0;
					return;
				}

				stack.resize(bp);

				auto& frame = callStack.back();
				ip = frame.ip + 1;
				bp = frame.bp;
				callStack.pop_back();

				push(retVal);
				break;
			}
			case Opcode::ReturnVoid: {
				stack.resize(bp);

				auto& frame = callStack.back();
				ip = frame.ip + 1;
				bp = frame.bp;
				callStack.pop_back();
				break;
			}
			case Opcode::AddI: {
				auto r = pop();
				auto l = pop();
				l.value.integer += r.value.integer;
				push(l);
				break;
			}
			case Opcode::AddF32: {
				auto r = pop();
				auto l = pop();
				l.value.f32 += r.value.f32;
				push(l);
				break;
			}
			case Opcode::AddF64: {
				auto r = pop();
				auto l = pop();
				l.value.f64 += r.value.f64;
				push(l);
				break;
			}
			case Opcode::SubI: {
				auto r = pop();
				auto l = pop();
				l.value.integer -= r.value.integer;
				push(l);
				break;
			}
			case Opcode::SubF32: {
				auto r = pop();
				auto l = pop();
				l.value.f32 -= r.value.f32;
				push(l);
				break;
			}
			case Opcode::SubF64: {
				auto r = pop();
				auto l = pop();
				l.value.f64 -= r.value.f64;
				push(l);
				break;
			}
			case Opcode::MulI: {
				auto r = pop();
				auto l = pop();
				l.value.integer *= r.value.integer;
				push(l);
				break;
			}
			case Opcode::MulF32: {
				auto r = pop();
				auto l = pop();
				l.value.f32 *= r.value.f32;
				push(l);
				break;
			}
			case Opcode::MulF64: {
				auto r = pop();
				auto l = pop();
				l.value.f64 *= r.value.f64;
				push(l);
				break;
			}
			case Opcode::DivI: {
				auto r = pop();
				auto l = pop();
				l.value.integer /= r.value.integer;
				push(l);
				break;
			}
			case Opcode::DivF32: {
				auto r = pop();
				auto l = pop();
				l.value.f32 /= r.value.f32;
				push(l);
				break;
			}
			case Opcode::DivF64: {
				auto r = pop();
				auto l = pop();
				l.value.f64 /= r.value.f64;
				push(l);
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
				push(VMValue(bool(l.value.f32 < r.value.f32)));
				break;
			}
			case Opcode::CmpLTF64: {
				auto r = pop();
				auto l = pop();
				push(VMValue(bool(l.value.f64 < r.value.f64)));
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
				push(VMValue(bool(l.value.f32 > r.value.f32)));
				break;
			}
			case Opcode::CmpGTF64: {
				auto r = pop();
				auto l = pop();
				push(VMValue(bool(l.value.f64 > r.value.f64)));
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
				std::cout << pop().value.f64;
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
				ip = pop().value.integer;
				break;
			}
			case Opcode::JmpIf: {
				auto newip = pop();
				auto cond = pop();
				if (cond) {
					ip = newip.value.integer;
				}
				break;
			}
			case Opcode::JmpIfNot: {
				auto newip = pop();
				auto cond = pop();
				if (!cond) {
					ip = newip.value.integer;
				}
				break;
			}
			case Opcode::New: {
				numallocs++;
				if ((numallocs % 1000) == 0) {
					gc.collect(stack);
				}
				auto type = read<uint16_t>();
				auto obj = gc.allocObject(chunk.types[type]);
				auto val = VMValue(obj);
				val.type = VMValue::Type::object;
				push(val);
				break;
			}
			case Opcode::Array: {
				numallocs++;
				if ((numallocs % 1000) == 0) {
					gc.collect(stack);
				}
				auto length = pop();
				auto type = pop();
				auto obj = gc.allocArray(chunk.types[type.value.integer], length.value.integer);
				auto val = VMValue(obj);
				val.type = VMValue::Type::object;
				push(val);
				break;
			}
			case Opcode::Ptr8: {
			case Opcode::Ptr16:
			case Opcode::Ptr32:
			case Opcode::Ptr64:
			case Opcode::ObjPtr64:
				auto obj = pop();
				if (!obj.value.object) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				auto offset = read<int8_t>();
				VMValue val((int64_t)0);
				switch ((Opcode)op) {
				case Opcode::Ptr8: memcpy(&val.value.integer, (char*)obj.value.object + offset, 1); break;
				case Opcode::Ptr16: memcpy(&val.value.integer, (char*)obj.value.object + offset, 2); break;
				case Opcode::Ptr32: memcpy(&val.value.integer, (char*)obj.value.object + offset, 4); break;
				case Opcode::Ptr64: memcpy(&val.value.integer, (char*)obj.value.object + offset, 8); break;
				case Opcode::ObjPtr64: memcpy(&val.value.integer, (char*)obj.value.object + offset, 8); break;
				default: exit(1);
				}
				val.type = (op == Opcode::ObjPtr64) ? VMValue::Type::object : VMValue::Type::integer;
				push(val);
				break;
			}
			case Opcode::Ptr64Var: {
			case Opcode::ObjPtr64Var:
				auto offset = read<int8_t>();
				auto var = read<int8_t>();
				auto obj = peek(bp + var).value.object;
				if (!obj) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				VMValue val((int64_t)0);
				memcpy(&val.value.integer, (char*)obj + offset, 8);
				val.type = (op == Opcode::ObjPtr64Var) ? VMValue::Type::object : VMValue::Type::integer;
				push(val);
				break;
			}
			case Opcode::PtrInd8: {
			case Opcode::PtrInd16:
			case Opcode::PtrInd32:
			case Opcode::PtrInd64:
			case Opcode::ObjPtrInd64:
				auto obj = pop();
				if (!obj) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				auto off = pop();
				auto off2 = read<int8_t>();
				VMValue val((int64_t)0);
				switch ((Opcode)op) {
				case Opcode::PtrInd8: memcpy(&val.value.integer, (char*)obj.value.object + off.value.integer + off2, 1); break;
				case Opcode::PtrInd16: memcpy(&val.value.integer, (char*)obj.value.object + off.value.integer + off2, 2); break;
				case Opcode::PtrInd32: memcpy(&val.value.integer, (char*)obj.value.object + off.value.integer + off2, 4); break;
				case Opcode::PtrInd64: memcpy(&val.value.integer, (char*)obj.value.object + off.value.integer + off2, 8); break;
				case Opcode::ObjPtrInd64: memcpy(&val.value.integer, (char*)obj.value.object + off.value.integer + off2, 8); break;
				default: exit(1);
				}
				val.type = (op == Opcode::ObjPtrInd64) ? VMValue::Type::object : VMValue::Type::integer;
				push(val);
				break;
			}
			case Opcode::StorePtr8: {
			case Opcode::StorePtr16:
			case Opcode::StorePtr32:
			case Opcode::StorePtr64:
				auto obj = pop();
				if (!obj.value.object) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				auto val = pop();
				auto offset = read<int8_t>();
				switch ((Opcode)op) {
				case Opcode::StorePtr8: memcpy((char*)obj.value.object + offset, &val.value.integer, 1); break;
				case Opcode::StorePtr16: memcpy((char*)obj.value.object + offset, &val.value.integer, 2); break;
				case Opcode::StorePtr32: memcpy((char*)obj.value.object + offset, &val.value.integer, 4); break;
				case Opcode::StorePtr64: memcpy((char*)obj.value.object + offset, &val.value.integer, 8); break;
				default: exit(1);
				}
				break;
			}
			case Opcode::StorePtr64Var: {
				auto val = pop();
				auto offset = read<int8_t>();
				auto var = read<int8_t>();
				auto obj = peek(bp + var).value.object;
				if (!obj) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				memcpy((char*)obj + offset, &val, 8);
				break;
			}
			case Opcode::StorePtrInd8: {
			case Opcode::StorePtrInd16:
			case Opcode::StorePtrInd32:
			case Opcode::StorePtrInd64:
				auto obj = pop();
				if (!obj.value.object) {
					std::cerr << "Null pointer access\n";
					std::cerr << printCallStack();
					exit(1);
				}
				auto off = pop();
				auto val = pop();
				auto off2 = read<int8_t>();
				switch ((Opcode)op) {
				case Opcode::StorePtrInd8: memcpy((char*)obj.value.object + off.value.integer + off2, &val.value.integer, 1); break;
				case Opcode::StorePtrInd16: memcpy((char*)obj.value.object + off.value.integer + off2, &val.value.integer, 2); break;
				case Opcode::StorePtrInd32: memcpy((char*)obj.value.object + off.value.integer + off2, &val.value.integer, 4); break;
				case Opcode::StorePtrInd64: memcpy((char*)obj.value.object + off.value.integer + off2, &val.value.integer, 8); break;
				default: exit(1);
				}
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
				if ((unsigned char)op >= numOpcodes) {
					std::cerr << "Opcode '" << (unsigned char)op << "' not implemented\n";
					std::cerr << printCallStack();
					exit(1);
				}
				else {
					std::cerr << "Opcode '" << opcodeInfo[(unsigned char)op].name << "' not implemented\n";
					std::cerr << printCallStack();
					exit(1);
				}
			}
		}
    }
	
    void VM::push(const VMValue& val) {
        stack.push_back(val);
    }

    VMValue VM::pop() {
        auto val = stack.back();
        stack.pop_back();
        return val;
    }

    void VM::pop(size_t num) {
        stack.resize(stack.size() - num);
    }

	VMValue VM::peek(size_t idx) {
		return stack[idx];
	}

	void VM::poke(size_t idx, const VMValue& val) {
		stack[idx] = val;
	}

	std::string VM::printCallStack() {
        std::stringstream sstr;
        Frame cur{bp, ip};
        int i = callStack.size();
		sstr << std::dec << (i+1) << "\n";
        while (i >= 0) {
            std::string name("???");

            size_t largest = 0;
            for (auto&& it: chunk.functions) {
                if (it.first >= largest && it.first <= cur.ip) {
                    name = it.second.name;
                    largest = it.first;
                }
            }

			std::string source = "";
			size_t line = 0;
			auto sourceLine = chunk.getLine(cur.ip);
			if (sourceLine) {
				source = chunk.files[sourceLine->file];
				line = sourceLine->line;
			}

			sstr << source << "\n";
			sstr << std::dec << line << "\n";
			sstr << name << " " << std::hex << std::setfill('0') << std::setw(8) << cur.ip << "\n";

            --i;
            if (i < 0 || callStack.empty()) break;
            cur = callStack[i];
        }
        return sstr.str();
    }
}