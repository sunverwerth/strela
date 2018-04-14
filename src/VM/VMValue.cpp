// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "VMValue.h"

#include <cstring>
#include <string>

namespace Strela {
    #define VMVALUE_OP(OP) \
    if (type == Type::integer) return VMValue(int64_t(value.integer OP other.value.integer)); \
    else if (type == Type::floating) return VMValue(double(value.floating OP other.value.floating)); \
    else if (type == Type::boolean) return VMValue(bool(value.boolean OP other.value.boolean)); \
    else return VMValue();

    #define VMVALUE_OP_LOG(OP) \
    if (type == Type::integer) return VMValue(bool(value.integer OP other.value.integer)); \
    else if (type == Type::floating) return VMValue(bool(value.floating OP other.value.floating)); \
    else if (type == Type::boolean) return VMValue(bool(value.boolean OP other.value.boolean)); \
    else return VMValue();

    VMValue::VMValue(): type(Type::null) {}
	VMValue::VMValue(int64_t val) : type(Type::integer) { value.integer = val; }
	VMValue::VMValue(double val) : type(Type::floating) { value.floating = val; }
	VMValue::VMValue(bool val) : type(Type::boolean) { value.boolean = val; }
	VMValue::VMValue(const char* val) : type(Type::string) { value.string = val; }
	VMValue::VMValue(VMObject* val) : type(Type::object) { value.object = val; }

    VMValue VMValue::operator==(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) == 0);
        }
        VMVALUE_OP_LOG(==);
    }

    VMValue VMValue::operator!=(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) != 0);
        }
        VMVALUE_OP_LOG(!=);
    }

    VMValue VMValue::operator<(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) < 0);
        }
        VMVALUE_OP_LOG(<);
    }

    VMValue VMValue::operator>(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) > 0);
        }
        VMVALUE_OP_LOG(>);
    }

    VMValue VMValue::operator<=(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) <= 0);
        }
        VMVALUE_OP_LOG(<=);
    }

    VMValue VMValue::operator>=(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            return VMValue(strcmp(value.string, other.value.string) >= 0);
        }
        VMVALUE_OP_LOG(>=);
    }

    VMValue VMValue::operator&&(const VMValue& other) const {
        VMVALUE_OP_LOG(&&);
    }

    VMValue VMValue::operator||(const VMValue& other) const {
        VMVALUE_OP_LOG(||);
    }

    
    VMValue VMValue::operator+(const VMValue& other) const {
        if (type == Type::string && other.type == Type::string) {
            auto len1 = strlen(value.string);
            auto len2 = strlen(other.value.string);
            auto str = new char[len1 + len2 + 1];
            str[len1 + len2] = 0;
            memcpy(&str[0], value.string, len1);
            memcpy(&str[len1], other.value.string, len2);
            return VMValue(str);
        }
        if (type == Type::string && other.type == Type::integer) {
            auto len1 = strlen(value.string);
            auto str = new char[len1 + 1 + 1];
            str[len1 + 1] = 0;
            memcpy(&str[0], value.string, len1);
            str[len1] = other.value.integer;
            return VMValue(str);
        }
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
		else if (type == Type::floating) return value.floating != 0;
		else if (type == Type::boolean) return value.boolean;
        else if (type == Type::null) return false;
        else if (type == Type::string) return true;
        else if (type == Type::object) return value.object != nullptr;
		else return false;
	}

    bool VMValue::equals(const VMValue& other) const {
        if (type != other.type) return false;

		if (type == Type::integer) return value.integer == other.value.integer;
		else if (type == Type::floating) return value.floating == other.value.floating;
		else if (type == Type::boolean) return value.boolean == other.value.boolean;
        else if (type == Type::null) return true;
        else if (type == Type::string) return value.string == other.value.string;
        else if (type == Type::object) return value.object == other.value.object;
		else return false;
    }

    std::string escape(const std::string&);

    std::string VMValue::dump() const {
        switch (type) {
            case Type::boolean: return std::string("bool: ") + (value.boolean ? "true" : "false");
            case Type::integer: return "int: " + std::to_string(value.integer);
            case Type::floating: return "float: " + std::to_string(value.floating);
            case Type::null: return "null";
            case Type::object: return "[object]";
            case Type::string: return "\"" + escape(value.string) + "\"";
        }
    }

    std::ostream& operator<<(std::ostream& str, const VMValue& v) {
        if (v.type == VMValue::Type::integer) {
            str.write("i", 1);
            str.write((const char*)&v.value.integer, 8);
        }
        else if (v.type == VMValue::Type::floating) {
            str.write("d", 1);
            str.write((const char*)&v.value.floating, 8);
        }
        else if (v.type == VMValue::Type::boolean) {
            str.write("b", 1);
            str.write((const char*)&v.value.boolean, 1);
        }
        else if (v.type == VMValue::Type::string) {
            str.write("s", 1);
            uint64_t len = strlen(v.value.string);
            str.write((const char*)&len, 8);
            str.write(v.value.string, len);
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
            str.read((char*)&v.value.floating, 8);
        }
        else if (t == 'b') {
            v.type = VMValue::Type::boolean;
            str.read((char*)&v.value.boolean, 1);
        }
        else if (t == 's') {
            uint64_t len;
            str.read((char*)&len, 8);
            char* string = new char[len + 1];
            string[len] = 0;
            str.read(string, len);
            v.type = VMValue::Type::string;
            v.value.string = string;
        }
        else if (t == 'n') {
            v.type = VMValue::Type::null;
        }
		
		return str;
    }

}