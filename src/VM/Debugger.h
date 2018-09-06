#ifndef Strela_VM_Debugger_h
#define Strela_VM_Debugger_h

#include "Opcode.h"

#include <map>
#include <deque>
#include <string>

namespace Strela {
    class VM;
    class VMType;

    struct Breakpoint {
        size_t address;
        Opcode originalOpcode;
        bool once;
    };
    
    class Debugger {
    public:
        Debugger(unsigned short port, VM& vm);
        ~Debugger();

        int run();
        
		void addBreakpoint(size_t address, bool once);
		void addBreakpoint(const std::string& file, size_t line, bool once);
		void removeBreakpoint(const std::string& file, size_t line);
		void removeBreakpoints(const std::string& file);
		void step();

    private:
        void write(const std::string& text);
        void write(const VMType& type, const void* val);

        int socket = -1;
        VM& vm;
        std::map<size_t, Breakpoint> breakpoints;
        std::deque<std::string> commands;
        std::string buffer;
    };
}

#endif