// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#ifndef Strela_VM_VMValue_h
#define Strela_VM_VMValue_h

#include <cstdint>
#include <iostream>

namespace Strela {
    class VMObject;
    
    struct VMValue {
        VMValue();
        explicit VMValue(int64_t val);
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
        std::string dump() const;

        union {
            int64_t integer;
            double floating;
            bool boolean;
            const char* string;
            VMObject* object;
        } value;

        enum class Type {
            null,
            integer,
            floating,
            boolean,
            string,
            object
        } type;
    };

    std::ostream& operator<<(std::ostream& str, const VMValue&);
    std::istream& operator>>(std::istream& str, VMValue&);

}

#endif