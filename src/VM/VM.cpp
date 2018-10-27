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
#include "../SourceFile.h"

#include <sstream>
#include <cstring>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <fstream>

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

	void VM::checkRead(const VMValue& val, int64_t offset) {
		if (val.type != VMValue::Type::object) {
			std::cerr << "Accessing non-object as object.\n";
			std::cerr << printCallStack();
			exit(1);
		}

		auto obj = val.value.object;
		if (!obj || (uint64_t)obj < 0xffff) {
			std::cerr << "Null pointer access\n";
			std::cerr << printCallStack();
			exit(1);
		}

		auto vmobject = (VMObject*)obj - 1;
		if (vmobject->type->isArray) {
			auto length = *(uint64_t*)obj;
			if (offset < 0 || offset > length * vmobject->type->arrayType->size + 8) {
				std::cerr << "Array access out of bounds.\n";
				std::cerr << printCallStack();
				exit(1);
			}
		}
		else if (vmobject->type->isObject) {
			auto length = vmobject->type->objectSize;
			if (offset < 0 || offset > length) {
				std::cerr << "Object access out of bounds.\n";
				std::cerr << printCallStack();
				exit(1);
			}
		}
	}

	void VM::checkWrite(const VMValue& val, int64_t offset) {
		checkRead(val, offset);
	}

#ifdef _WIN32
	#include <WinSock2.h>
#endif

	std::ofstream sampleFile;

    VM::VM(ByteCodeChunk& chunk, const std::vector<std::string>& arguments): chunk(chunk), status(RUNNING) {
#ifdef _WIN32
		auto mod = LoadLibrary("msvcrt.dll");
		auto sockmod = LoadLibrary("ws2_32.dll");
		auto kernmod = LoadLibrary("kernel32.dll");
		WSADATA wsadata{ 0 };
		WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
		sampleFile.open("flamegraph.json", std::ios::binary);

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
			if (!ff.ptr) {
				ff.ptr = ForeignFunction::callback(GetProcAddress(kernmod, ff.name.c_str()));
			}
#else
			ff.ptr = ForeignFunction::callback(dlsym(RTLD_DEFAULT, ff.name.c_str()));
#endif
			
			if (!ff.ptr) {
				std::cerr << "Unresolved external symbol \"" << ff.name << "\"\n";
				exit(1);
			}
		}

        VMType* arrtype = nullptr;
        VMType* strtype = nullptr;
        VMType* u8type = nullptr;
        for (auto&& type: chunk.types) {
            if (type->name == "String") {
                strtype = type;
            }
            else if (type->name == "String[]") {
                arrtype = type;
            }
            else if (type->name == "u8[]") {
                u8type = type;
            }
        }

		// init string constants
		for (auto& constant: chunk.constants) {
            if (constant.type == VMValue::Type::object) {
                auto len = strlen((char*)constant.value.object);
                auto string = gc.allocObject(strtype);
                gc.lock(string);
                auto arr = gc.allocArray(u8type, len + 1);

                memcpy((char*)arr + 8, constant.value.object, len);
                ((char*)arr)[len + 8] = 0;

                *(void**)string = arr;

                constant.value.object = string;
            }
		}

        ip = chunk.main;
        bp = 0;

		auto arr = gc.allocArray(arrtype, arguments.size());
		auto data = (char*)arr;
		data += 8;
		for (int i = 0; i < arguments.size(); ++i) {
			auto len = arguments[i].length();
			auto string = gc.allocObject(strtype);
			auto arr = gc.allocArray(u8type, len + 1);

			memcpy((char*)arr + 8, arguments[i].c_str(), len);
			((char*)arr)[len + 8] = 0;

			*(void**)string = arr;
			*(void**)data = string;

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
				callStack.push_back({ bp, ip });
				bp = stack.size() - numargs;
				ip = newip;
				break;
			}
			case Opcode::CallImm: {
				auto newip = read<uint32_t>();
				auto numargs = read<uint8_t>();
				callStack.push_back({ bp, ip });
				bp = stack.size() - numargs;
				ip = newip;
				break;
			}
			case Opcode::F32tI64: {
				auto& back = stack.back();
				back.value.integer = back.value.f32;
				back.type = VMValue::Type::integer;
				break;
			}
			case Opcode::F64tI64: {
				auto& back = stack.back();
				back.value.integer = back.value.f64;
				back.type = VMValue::Type::integer;
				break;
			}
			case Opcode::I64tF32: {
				auto& back = stack.back();
				back.value.integer = 0;
				back.value.f32 = back.value.integer;
				back.type = VMValue::Type::floating;
				break;
			}
			case Opcode::I64tF64: {
				auto& back = stack.back();
				back.value.f64 = back.value.integer;
				back.type = VMValue::Type::floating;
				break;
			}
			case Opcode::F64tF32: {
				auto& back = stack.back();
				back.value.integer = 0;
				back.value.f32 = back.value.f64;
				break;
			}
			case Opcode::F32tF64: {
				auto& back = stack.back();
				back.value.f64 = back.value.f32;
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
                        if (aptr) {
                            auto obj = (VMObject*)aptr - 1;
                            if (obj->type->name == "String") {
                                aptr = *(uint64_t**)(obj + 1) + 1;
                            }
                        }
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
			case Opcode::BuiltinCall: {
				auto func = (BuiltinFunction)read<uint64_t>();
				func(*this);
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
				ip = frame.ip;
				bp = frame.bp;
				callStack.pop_back();

				push(retVal);
				break;
			}
			case Opcode::ReturnVoid: {
				stack.resize(bp);

				auto& frame = callStack.back();
				ip = frame.ip;
				bp = frame.bp;
				callStack.pop_back();
				break;
			}
			case Opcode::AddI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.integer += r.value.integer;
				break;
			}
			case Opcode::AddF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f32 += r.value.f32;
				break;
			}
			case Opcode::AddF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f64 += r.value.f64;
				break;
			}
			case Opcode::SubI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.integer -= r.value.integer;
				break;
			}
			case Opcode::SubF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f32 -= r.value.f32;
				break;
			}
			case Opcode::SubF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f64 -= r.value.f64;
				break;
			}
			case Opcode::MulI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.integer *= r.value.integer;
				break;
			}
			case Opcode::MulF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f32 *= r.value.f32;
				break;
			}
			case Opcode::MulF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f64 *= r.value.f64;
				break;
			}
			case Opcode::DivI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.integer /= r.value.integer;
				break;
			}
			case Opcode::DivF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f32 /= r.value.f32;
				break;
			}
			case Opcode::DivF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.f64 /= r.value.f64;
				break;
			}
            case Opcode::ModI: {
                auto r = pop();
				auto& l = stack.back();
                l.value.integer %= r.value.integer;
                break;
            }
			case Opcode::CmpEQ: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = (l == r);
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpNE: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = (l != r);
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpLTI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.integer < r.value.integer;
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpLTF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.f32 < r.value.f32;
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpLTF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.f64 < r.value.f64;
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpGTI: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.integer > r.value.integer;
				l.type = VMValue::Type::boolean;
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpGTF32: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.f32 > r.value.f32;
				l.type = VMValue::Type::boolean;
				break;
			}
			case Opcode::CmpGTF64: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.f64 > r.value.f64;
				l.type = VMValue::Type::boolean;
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
				auto& l = stack.back();
				l.value.boolean = l.value.boolean && r.value.boolean;
				break;
			}
			case Opcode::OrL: {
				auto r = pop();
				auto& l = stack.back();
				l.value.boolean = l.value.boolean || r.value.boolean;
				break;
			}
			case Opcode::Not: {
				auto& l = stack.back();
				l.value.boolean = !l.value.boolean;
				break;
			}
			case Opcode::PrintI: {
				std::cout << pop().value.integer;
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintF32: {
				float f;
				auto v = pop();
				memcpy(&f, &v, sizeof(float));
				std::cout << f;
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintF64: {
				std::cout << pop().value.f64;
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintN: {
				std::cout << "(null)";
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintS: {
				auto val = pop();
				std::cout << (*(char**)val.value.object + 8);
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintO: {
				std::cout << "[object]";
				std::flush(std::cout);
				break;
			}
			case Opcode::PrintB: {
				std::cout << (pop().value.boolean ? "true" : "false");
				std::flush(std::cout);
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
				auto v = pop();
				auto obj = v.value.object;
				auto offset = read<int8_t>();
				VMValue val((int64_t)0);
#ifdef _DEBUG
				checkRead(v, offset);
#endif

				switch ((Opcode)op) {
				case Opcode::Ptr8: memcpy(&val.value.integer, (char*)obj + offset, 1); break;
				case Opcode::Ptr16: memcpy(&val.value.integer, (char*)obj + offset, 2); break;
				case Opcode::Ptr32: memcpy(&val.value.integer, (char*)obj + offset, 4); break;
				case Opcode::Ptr64: memcpy(&val.value.integer, (char*)obj + offset, 8); break;
				case Opcode::ObjPtr64: memcpy(&val.value.integer, (char*)obj + offset, 8); break;
				default: exit(1);
				}
				val.type = (op == Opcode::ObjPtr64) ? VMValue::Type::object : VMValue::Type::integer;
				push(val);
				break;
			}
			case Opcode::Ptr64Var: {
			case Opcode::ObjPtr64Var:
				auto constOffset = read<int8_t>();
				auto var = read<int8_t>();
				auto v = peek(bp + var);
				auto obj = v.value.object;
				VMValue val((int64_t)0);

#ifdef _DEBUG
				checkRead(v, constOffset);
#endif

				memcpy(&val.value.integer, (char*)obj + constOffset, 8);
				val.type = (op == Opcode::ObjPtr64Var) ? VMValue::Type::object : VMValue::Type::integer;
				push(val);
				break;
			}
			case Opcode::PtrInd8: {
			case Opcode::PtrInd16:
			case Opcode::PtrInd32:
			case Opcode::PtrInd64:
			case Opcode::ObjPtrInd64:
				auto v = pop();
				auto obj = v.value.object;
				auto offset = pop().value.integer;
				auto constOffset = read<int8_t>();

#ifdef _DEBUG
				checkRead(v, offset + constOffset);
#endif

				VMValue val((int64_t)0);
				switch ((Opcode)op) {
				case Opcode::PtrInd8: memcpy(&val.value.integer, (char*)obj + offset + constOffset, 1); break;
				case Opcode::PtrInd16: memcpy(&val.value.integer, (char*)obj + offset + constOffset, 2); break;
				case Opcode::PtrInd32: memcpy(&val.value.integer, (char*)obj + offset + constOffset, 4); break;
				case Opcode::PtrInd64: memcpy(&val.value.integer, (char*)obj + offset + constOffset, 8); break;
				case Opcode::ObjPtrInd64: memcpy(&val.value.integer, (char*)obj + offset + constOffset, 8); break;
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
				auto v = pop();
				auto obj = v.value.object;
				auto val = pop();
				auto offset = read<int8_t>();

#ifdef _DEBUG
				checkWrite(v, offset);
#endif

				switch ((Opcode)op) {
				case Opcode::StorePtr8: memcpy((char*)obj + offset, &val.value.integer, 1); break;
				case Opcode::StorePtr16: memcpy((char*)obj + offset, &val.value.integer, 2); break;
				case Opcode::StorePtr32: memcpy((char*)obj + offset, &val.value.integer, 4); break;
				case Opcode::StorePtr64: memcpy((char*)obj + offset, &val.value.integer, 8); break;
				default: exit(1);
				}
				break;
			}
			case Opcode::StorePtr64Var: {
				auto val = pop();
				auto offset = read<int8_t>();
				auto var = read<int8_t>();
				auto v = peek(bp + var);
				auto obj = v.value.object;

#ifdef _DEBUG
				checkWrite(v, offset);
#endif

				memcpy((char*)obj + offset, &val, 8);
				break;
			}
			case Opcode::StorePtrInd8: {
			case Opcode::StorePtrInd16:
			case Opcode::StorePtrInd32:
			case Opcode::StorePtrInd64:
				auto v = pop();
				auto obj = v.value.object;
				auto offset = pop().value.integer;
				auto val = pop();
				auto constOffset = read<int8_t>();

#ifdef _DEBUG
				checkWrite(v, offset + constOffset);
#endif

				switch ((Opcode)op) {
				case Opcode::StorePtrInd8: memcpy((char*)obj + offset + constOffset, &val.value.integer, 1); break;
				case Opcode::StorePtrInd16: memcpy((char*)obj + offset + constOffset, &val.value.integer, 2); break;
				case Opcode::StorePtrInd32: memcpy((char*)obj + offset + constOffset, &val.value.integer, 4); break;
				case Opcode::StorePtrInd64: memcpy((char*)obj + offset + constOffset, &val.value.integer, 8); break;
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
            case Opcode::CmpType: {
                auto v = pop();
#ifdef _DEBUG
                checkRead(v, 0);
#endif
                auto obj = (VMObject*)v.value.object - 1;
				auto typeIndex = read<uint64_t>();

#ifdef _DEBUG
				if (typeIndex >= chunk.types.size()) {
					std::cerr << "Type index out of range.\n";
					std::cerr << printCallStack();
					exit(1);
				}
#endif
				push(VMValue(chunk.types[typeIndex] == obj->type));

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
				source = chunk.files[sourceLine->file]->filename;
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

	void VM::writeSample() {

		Frame cur{ bp, ip };
		int i = callStack.size();
		sampleFile << "[\n";
		while (i >= 0) {
			std::string name("???");

			size_t largest = 0;
			for (auto&& it : chunk.functions) {
				if (it.first >= largest && it.first <= cur.ip) {
					name = it.second.name;
					largest = it.first;
				}
			}

			sampleFile << "\"" << name << "\",\n";

			--i;
			if (i < 0 || callStack.empty()) break;
			cur = callStack[i];
		}
		sampleFile << "],\n";
	}
}