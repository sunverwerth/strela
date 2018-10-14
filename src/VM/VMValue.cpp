// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "VMValue.h"
#include "VMObject.h"

#include <string>

namespace Strela {
    #define VMVALUE_OP(OP) \
    if (type == Type::integer) return VMValue(int64_t(value.integer OP other.value.integer)); \
    else if (type == Type::floating) return VMValue(double(value.f64 OP other.value.f64)); \
    else if (type == Type::boolean) return VMValue(bool(value.boolean OP other.value.boolean)); \
    else return VMValue();

    #define VMVALUE_OP_LOG(OP) \
    if (type == Type::integer) return VMValue(bool(value.integer OP other.value.integer)); \
    else if (type == Type::floating) return VMValue(bool(value.f64 OP other.value.f64)); \
    else if (type == Type::boolean) return VMValue(bool(value.boolean OP other.value.boolean)); \
    else return VMValue();

    VMValue::VMValue(): type(Type::null) { value.integer = 0; }
	VMValue::VMValue(int64_t val) : type(Type::integer) { value.integer = val; }
	VMValue::VMValue(double val) : type(Type::floating) { value.f64 = val; }
	VMValue::VMValue(bool val) : type(Type::boolean) { value.boolean = val; }
	VMValue::VMValue(void* val) : type(Type::object) { value.object = val; }

    VMValue VMValue::operator==(const VMValue& other) const {
        VMVALUE_OP_LOG(==);
    }

    VMValue VMValue::operator!=(const VMValue& other) const {
        VMVALUE_OP_LOG(!=);
    }

    VMValue VMValue::operator<(const VMValue& other) const {
        VMVALUE_OP_LOG(<);
    }

    VMValue VMValue::operator>(const VMValue& other) const {
        VMVALUE_OP_LOG(>);
    }

    VMValue VMValue::operator<=(const VMValue& other) const {
        VMVALUE_OP_LOG(<=);
    }

    VMValue VMValue::operator>=(const VMValue& other) const {
        VMVALUE_OP_LOG(>=);
    }

    VMValue VMValue::operator&&(const VMValue& other) const {
        VMVALUE_OP_LOG(&&);
    }

    VMValue VMValue::operator||(const VMValue& other) const {
        VMVALUE_OP_LOG(||);
    }

    
    VMValue VMValue::operator+(const VMValue& other) const {
        VMVALUE_OP(+);
    }

    VMValue VMValue::operator-(const VMValue& other) const {
        VMVALUE_OP(-);
    }

    VMValue VMValue::operator*(const VMValue& other) const {
        VMVALUE_OP(*);
    }

    VMValue VMValue::operator/(const VMValue& other) const {
        VMVALUE_OP(/);
    }

    VMValue VMValue::operator!() const {
        return VMValue(!bool(*this));
    }

	VMValue::operator bool() const {
		if (type == Type::integer) return value.integer != 0;
		else if (type == Type::floating) return value.f64 != 0;
		else if (type == Type::boolean) return value.boolean;
        else if (type == Type::null) return false;
        else if (type == Type::object) return value.object != nullptr;
		else return false;
	}

    bool VMValue::equals(const VMValue& other) const {
        if (type != other.type) return false;

		if (type == Type::integer) return value.integer == other.value.integer;
		else if (type == Type::floating) return value.f64 == other.value.f64;
		else if (type == Type::boolean) return value.boolean == other.value.boolean;
        else if (type == Type::null) return true;
        else if (type == Type::object) return value.object == other.value.object;
		else return false;
    }

    std::string VMValue::dump() const {
        switch (type) {
            case Type::boolean: return std::string("bool: ") + (value.boolean ? "true" : "false");
            case Type::integer: return "int: " + std::to_string(value.integer);
            case Type::floating: return "float: " + std::to_string(value.f64);
            case Type::null: return "null";
            case Type::object: return "[object]";
        }
    }

    struct StringConst {
        bool marked;
        VMType* type;
        uint64_t* str;
        uint64_t len;
        char chars[];
    };

    std::ostream& operator<<(std::ostream& str, const VMValue& v) {
        if (v.type == VMValue::Type::integer) {
            str.write("i", 1);
            str.write((const char*)&v.value.integer, 8);
        }
        else if (v.type == VMValue::Type::floating) {
            str.write("d", 1);
            str.write((const char*)&v.value.f64, 8);
        }
        else if (v.type == VMValue::Type::boolean) {
            str.write("b", 1);
            str.write((const char*)&v.value.boolean, 1);
        }
        else if (v.type == VMValue::Type::object) {
            str.write("s", 1);
            /*StringConst* string = v.value.object;
            uint64_t len = strlen(v.value.string);
            str.write((const char*)&len, 8);
            str.write(v.value.string, len);*/
        }
        else if (v.type == VMValue::Type::null) {
            str.write("n", 1);
        }

		return str;
    }

    std::istream& operator>>(std::istream& str, VMValue& v) {
        char t;
        str.read(&t, 1);

        if (t == 'i') {
            v.type = VMValue::Type::integer;
            str.read((char*)&v.value.integer, 8);
        }
        else if (t == 'd') {
            v.type = VMValue::Type::floating;
            str.read((char*)&v.value.f64, 8);
        }
        else if (t == 'b') {
            v.type = VMValue::Type::boolean;
            str.read((char*)&v.value.boolean, 1);
        }
        else if (t == 's') {
            uint64_t len;
            str.read((char*)&len, 8);

            struct StringConst {
                bool marked;
                VMType* type;
                uint64_t* str;
                uint64_t len;
                char chars[];
            };

            StringConst* string = (StringConst*)calloc(1, sizeof(StringConst) + len + 1 );
            
            str.read(string->chars, len);
            string->chars[len] = 0;
            string->marked = true;
            string->type = nullptr;
            string->str = &string->len;

            v.type = VMValue::Type::object;
            v.value.object = string;
        }
        else if (t == 'n') {
            v.type = VMValue::Type::null;
        }
		
		return str;
    }

}