#ifndef Strela_VMValue_h
#define Strela_VMValue_h

#include <cstdint>
#include <iostream>

namespace Strela {
    class Type;
    class VMObject;
    
    struct VMValue {
        VMValue();
        explicit VMValue(uint8_t val);
        explicit VMValue(uint16_t val);
        explicit VMValue(uint32_t val);
        explicit VMValue(uint64_t val);
        explicit VMValue(int8_t val);
        explicit VMValue(int16_t val);
        explicit VMValue(int32_t val);
        explicit VMValue(int64_t val);
        explicit VMValue(float val);
        explicit VMValue(double val);
        explicit VMValue(const char* val);
        explicit VMValue(bool val);
        explicit VMValue(VMObject* val);

        VMValue operator==(const VMValue& other) const;
        VMValue operator!=(const VMValue& other) const;
        VMValue operator<(const VMValue& other) const;
        VMValue operator>(const VMValue& other) const;
        VMValue operator<=(const VMValue& other) const;
        VMValue operator>=(const VMValue& other) const;
        VMValue operator&&(const VMValue& other) const;
        VMValue operator||(const VMValue& other) const;
        
        VMValue operator+(const VMValue& other) const;
        VMValue operator-(const VMValue& other) const;
        VMValue operator*(const VMValue& other) const;
        VMValue operator/(const VMValue& other) const;

        VMValue operator!() const;
		operator bool() const;

        bool equals(const VMValue& other) const;

        enum class Type {
            null,
            u8, u16, u32, u64,
            i8, i16, i32, i64,
            f32, f64,
            boolean,
            string,
            object
        } type;

        union {
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            float f32;
            double f64;
            bool boolean;
            const char* string;
            VMObject* object;
        } value;
    };

    std::ostream& operator<<(std::ostream& str, const VMValue&);
    std::istream& operator>>(std::istream& str, VMValue&);

}

#endif