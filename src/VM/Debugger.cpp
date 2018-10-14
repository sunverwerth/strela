#include "Debugger.h"
#include "VM.h"
#include "ByteCodeChunk.h"
#include "../utils.h"
#include "../SourceFile.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
typedef int socklen_t;
#define SHUT_RDWR SD_BOTH
void close(int sock) {
	closesocket(sock);
}
void usleep(int us) {
	Sleep(us / 1000);
}
int ioctl(int socket, long cmd, int* argp) {
	return ioctlsocket(socket, cmd, (u_long*)argp);
}

#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <sstream>
#include <iomanip>

namespace Strela {
	bool pathEquals(const std::string& a, const std::string& b) {
		if (a.length() != b.length()) return false;
		for (int i = 0; i < a.length(); ++i) {
			if (tolower(a[i]) == tolower(b[i])) continue;
			if (a[i] == '/' && b[i] == '\\') continue;
			if (a[i] == '\\' && b[i] == '/') continue;
			return false;
		}
		return true;
	}

	Debugger::Debugger(unsigned short port, VM& vm) : vm(vm) {
		vm.status = VM::STOPPED;
#ifdef _WIN32
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
		socket = ::socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr{ 0 };
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
		auto res = connect(socket, (sockaddr*)&addr, sizeof(addr));
	}

	Debugger::~Debugger() {
		shutdown(socket, SHUT_RDWR);
		close(socket);
#ifdef _WIN32
		WSACleanup();
#endif
	}

	void Debugger::write(const VMType& type, const void* val) {
		write(type.name + "\n");

		if (type.name == "int" || type.name == "i64") {
			write("data\n");
			write(std::to_string(*(int64_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "i32") {
			write("data\n");
			write(std::to_string(*(int32_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "i16") {
			write("data\n");
			write(std::to_string(*(int16_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "i8") {
			write("data\n");
			write(std::to_string(*(int8_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "u64") {
			write("data\n");
			write(std::to_string(*(uint64_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "u32") {
			write("data\n");
			write(std::to_string(*(uint32_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "u16") {
			write("data\n");
			write(std::to_string(*(uint16_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "u8") {
			write("data\n");
			write(std::to_string(*(uint8_t*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "float" || type.name == "f64") {
			write("data\n");
			write(std::to_string(*(double*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "f32") {
			write("data\n");
			write(std::to_string(*(float*)val) + "\n");
			write("0\n");
		}
		else if (type.name == "bool") {
			write("data\n");
			write(*(bool*)val ? "true\n" : "false\n");
			write("0\n");
		}
		else if (type.name == "null") {
			write("data\n");
			write("(null)\n");
			write("0\n");
		}
		else if (type.name == "String") {
			write("class\n");
			if (*(char**)val) {
				write("\"" + escape(**((char***)val) + 8) + "\"\n");
			}
			else {
				write("String (null)\n");
			}
			write("0\n");
		}
		else if (type.isEnum) {
			write("enum\n");
			auto ev = *(uint32_t*)val;
			if (ev < type.enumValues.size()) {
				write(type.name + "." + type.enumValues[ev] + "\n");
			}
			else {
				write(std::to_string(ev) + "\n");
			}
			write("0\n");
		}
		else if (type.isArray) {
			write("array\n");
			auto addr = *(uint64_t**)val;
			if (addr != 0) {
				write(type.name + " length=" + std::to_string(*addr) + "\n");
			}
			else {
				write(type.name + " (null)\n");
			}
			write(std::to_string((size_t)addr) + "\n");
		}
		else if (type.isObject) {
			write("class\n");
			auto addr = *(size_t*)val;
			if (addr != 0) {
				write(type.name + "\n");
			}
			else {
				write(type.name + " (null)\n");
			}
			write(std::to_string(addr) + "\n");
		}
		else {
			write("data\n");
			write(type.name + "\n");
			write("0\n");
		}
	}

	int Debugger::run() {
		do {
			int num;
			ioctl(socket, FIONREAD, &num);
			while (num--) {
				char ch;
				recv(socket, &ch, 1, 0);
				if (ch == '\n') {
					//std::cerr << buffer << "\n";
					commands.push_back(buffer);
					buffer.clear();
				}
				else if (ch != '\r') {
					buffer += ch;
				}
			}

			// ADD\nfilename\n10\n - Add breakpoint at line 10 in filename
			// REMOVE\nfilename\n10\n - Remove breakpoint at line 10 in filename
			// PAUSE\n - Add breakpoint at current position
			// STACK\n - get current stack
			// STEP\n - step to next line
			// STEPIN\n - step into function or to next line
			// STEPOUT\n - step out of current function
			// CONTINUE\n - continue program
			// DISCONNECT\n - disconnect socket and terminate

			if (commands.size() > 0) {
				if (commands.front() == "ADD") {
					if (commands.size() >= 3) {
						commands.pop_front();
						auto file = commands.front();
						commands.pop_front();
						auto line = commands.front();
						commands.pop_front();
						addBreakpoint(file, std::stoi(line), false);
						write("ACK_ADD\n");
					}
				}
				else if (commands.front() == "ADD_ALL" || commands.front() == "ADD_ALL_START") {
					auto cmd = commands.front();
					if (commands.size() >= 2) {
						auto num = std::stoi(commands[1]);
						if (commands.size() >= 3 + num) {
							commands.pop_front();
							commands.pop_front(); // num
							auto file = commands.front();
							commands.pop_front();

							removeBreakpoints(file);

							if (cmd == "ADD_ALL") {
								write("ACK_ADD_ALL\n");
								write(std::to_string(num) + "\n");
							}
							for (int i = 0; i < num; ++i) {
								auto line = std::stoi(commands.front());
								addBreakpoint(file, line, false);
								commands.pop_front();
								if (cmd == "ADD_ALL") write(std::to_string(line) + "\n");
							}
						}
					}
				}
				else if (commands.front() == "REMOVE") {
					if (commands.size() >= 3) {
						commands.pop_front();
						auto file = commands.front();
						commands.pop_front();
						auto line = commands.front();
						commands.pop_front();
						removeBreakpoint(file, std::stoi(line));
						write("ACK_REMOVE\n");
					}
				}
				else if (commands.front() == "PAUSE") {
					commands.pop_front();
					vm.status = VM::STOPPED;
					write("ACK_PAUSE\n");
					write("HIT\n");
				}
				else if (commands.front() == "STACK") {
					commands.pop_front();
					write("ACK_STACK\n");
					write(vm.printCallStack());
				}
				else if (commands.front() == "CONTINUE") {
					commands.pop_front();
					step();
					vm.status = VM::RUNNING;
					write("ACK_CONTINUE\n");
				}
				else if (commands.front() == "START") {
					// sent upon first start, no ack
					commands.pop_front();
					vm.status = VM::RUNNING;
				}
				else if (commands.front() == "DISCONNECT") {
					close(socket);
					exit(1);
				}
				else if (commands.front() == "THREADS") {
					commands.pop_front();
					write("ACK_THREADS\n");
					write("1\n");
					write("Main Thread\n");
				}
				else if (commands.front() == "VARIABLES") {
					if (commands.size() >= 2) {
						commands.pop_front();
						size_t memref = std::stoul(commands[0]);
						commands.pop_front();
						write("ACK_VARIABLES\n");

						if (memref <= vm.callStack.size() + 1) {
							size_t bp = vm.bp;
							size_t ip = vm.ip;
							if (memref > 1 && memref < vm.callStack.size() + 2) {
								bp = vm.callStack[vm.callStack.size() + 1 - memref].bp;
								ip = vm.callStack[vm.callStack.size() + 1 - memref].ip;
							}

							FunctionInfo* fi = nullptr;
							size_t largest = 0;
							for (auto&& it : vm.chunk.functions) {
								if (it.first >= largest && it.first <= ip) {
									fi = &it.second;
									largest = it.first;
								}
							}

							if (!fi) {
								write("0\n");
							}
							else {
								int numVars = 0;
								for (auto& var : fi->variables) {
									if (bp + var.offset >= vm.stack.size()) {
										continue;
									}
									numVars++;
								}
								write(std::to_string(numVars) + "\n");
								for (auto& var : fi->variables) {
									if (bp + var.offset >= vm.stack.size()) {
										continue;
									}

									write(var.name + "\n");
									write(*var.type, &vm.stack[bp + var.offset]);
								}
							}
						}
						else {
							auto obj = (VMObject*)memref;
							obj--;

							if (obj->type->isArray) {
								uint64_t len = *(uint64_t*)memref;
								write(std::to_string(len + 1) + "\n");
								write("length\n");
								write("u64\n");
								write("data\n");
								write(std::to_string(len) + "\n");
								write("0\n");
								char* data = (char*)memref + 8;
								for (uint64_t i = 0; i < len; ++i) {
									write(std::to_string(i) + "\n");
									write(*obj->type->arrayType, data);
									data += obj->type->arrayType->size;
								}
							}
							else {
								if (obj->type->unionTypes.size()) {
									VMType* type = obj->type->unionTypes[*(uint64_t*)memref];
									write("1\n");
									write("value\n");
									auto ref = (void*)(memref + 8);
									write(*type, ref);
								}
								else {
									write(std::to_string(obj->type->fields.size()) + "\n");
									for (auto& field : obj->type->fields) {
										write(field.name + "\n");
										write(*field.type, (char*)memref + field.offset);
									}
								}
							}
						}
					}
				}
				else if (commands.front() == "STEP") {
					commands.pop_front();

					write("ACK_STEP\n");

					auto line = vm.chunk.getLine(vm.ip);

					do {
						auto op = vm.chunk.opcodes[vm.ip];

						if (op == Opcode::Trap) {
							auto it = breakpoints.find(vm.ip);
							if (it != breakpoints.end()) {
								op = it->second.originalOpcode;
							}
						}

						if (op == Opcode::Call) {
							addBreakpoint(vm.ip + 2, true);
							step();
							vm.status = VM::RUNNING;
							break;
						}
						else if (op == Opcode::CallImm) {
							addBreakpoint(vm.ip + 6, true);
							step();
							vm.status = VM::RUNNING;
							break;
						}

						step();

						auto line2 = vm.chunk.getLine(vm.ip);
						if (!line || !line2 || line->line != line2->line) {
							write("HIT\n");
							vm.status = VM::STOPPED;
							break;
						}
					} while (true);
				}
				else if (commands.front() == "STEPIN") {
					commands.pop_front();

					write("ACK_STEPIN\n");

					auto line = vm.chunk.getLine(vm.ip);

					do {
						auto op = vm.chunk.opcodes[vm.ip];

						if (op == Opcode::Trap) {
							auto it = breakpoints.find(vm.ip);
							if (it != breakpoints.end()) {
								op = it->second.originalOpcode;
							}
						}

						if (op == Opcode::Call || op == Opcode::CallImm) {
							step();
							write("HIT\n");
							vm.status = VM::STOPPED;
							break;
						}

						step();

						auto line2 = vm.chunk.getLine(vm.ip);
						if (!line || !line2 || line->line != line2->line) {
							write("HIT\n");
							vm.status = VM::STOPPED;
							break;
						}
					} while (true);
				}
				else if (commands.front() == "STEPOUT") {
					commands.pop_front();

					write("ACK_STEPOUT\n");

					do {
						auto op = vm.chunk.opcodes[vm.ip];

						if (op == Opcode::Trap) {
							auto it = breakpoints.find(vm.ip);
							if (it != breakpoints.end()) {
								op = it->second.originalOpcode;
							}
						}

						if (op == Opcode::Return || op == Opcode::ReturnVoid) {
							step();
							write("HIT\n");
							vm.status = VM::STOPPED;
							break;
						}

						step();
					} while (vm.status != VM::FINISHED);
				}
				else {
					commands.pop_front();
				}
			}

			if (vm.status == VM::RUNNING) {
				// do max 0xffffff opcodes
#ifdef _DEBUG
				vm.step(1000);
#else
				vm.step(0xffffff);
#endif
				if (vm.status == VM::STOPPED) {
					write("HIT\n");
				}
			}
			else {
				usleep(1000);
			}
		} while (vm.status != VM::FINISHED);

		return vm.exitCode.value.integer;
	}

	void Debugger::write(const std::string& str) {
		//std::cerr << ">>> " << str;
		auto size = str.size();
		while (size) {
			size -= send(socket, str.data(), size, 0);
		}
	}

	void Debugger::addBreakpoint(size_t address, bool once) {
		if (vm.chunk.opcodes[address] == Opcode::Trap) {
			return;
		}

		breakpoints.insert(std::make_pair(address, Breakpoint{ address, vm.chunk.opcodes[address], once }));
		vm.chunk.opcodes[address] = Opcode::Trap;
	}

	void Debugger::addBreakpoint(const std::string& file, size_t line, bool once) {
		for (auto& codeline : vm.chunk.lines) {
			if (pathEquals(vm.chunk.files[codeline.file]->filename, file) && codeline.line == line) {
				addBreakpoint(codeline.address, once);
				return;
			}
		}
	}

	void Debugger::removeBreakpoint(const std::string& file, size_t line) {
		for (auto& codeline : vm.chunk.lines) {
			if (pathEquals(vm.chunk.files[codeline.file]->filename, file) && codeline.line == line) {
				auto it = breakpoints.find(codeline.address);
				if (it != breakpoints.end()) {
					vm.chunk.opcodes[codeline.address] = it->second.originalOpcode;
					breakpoints.erase(it);
					write("Removed breakpoint\n");
					return;
				}
			}
		}
	}

	void Debugger::removeBreakpoints(const std::string& file) {
		for (auto it = breakpoints.begin(); it != breakpoints.end(); ) {
			if (auto source = vm.chunk.getLine(it->second.address)) {
				if (pathEquals(vm.chunk.files[source->file]->filename, file)) {
					vm.chunk.opcodes[source->address] = it->second.originalOpcode;
					it = breakpoints.erase(it);
				}
				else {
					it++;
				}
			}
			else {
				it++;
			}
		}
	}

	void Debugger::step() {
		if (vm.chunk.opcodes[vm.ip] != Opcode::Trap) {
			vm.step(1);
			return;
		}

		auto it = breakpoints.find(vm.ip);
		if (it == breakpoints.end()) {
			// No breakpoint found, treat 'Trap' as NOP
			vm.ip++;
			return;
		}

		vm.chunk.opcodes[vm.ip] = it->second.originalOpcode;
		vm.step(1);

		if (it->second.once) {
			breakpoints.erase(it);
		}
		else {
			vm.chunk.opcodes[it->second.address] = Opcode::Trap;
		}
	}
}