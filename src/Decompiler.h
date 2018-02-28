#ifndef Strela_Decompiler_h
#define Strela_Decompiler_h

#include <cstddef>
#include <cstdint>

namespace Strela {
    class ByteCodeChunk;
    class VMValue;

    class Decompiler {
    public:
        Decompiler(const ByteCodeChunk& chunk): chunk(chunk) {}

        void listing() const;
        uint64_t getArg(size_t pos) const;

    private:
        void printValue(const VMValue& v) const;

    private:
        const ByteCodeChunk& chunk;
    };
}

#endif