#include "VMValue.h"

#include <cstring>

namespace Strela {
    #define VMVALUE_OP(OP) \
    if (type == Type::u8) return VMValue(value.u8 OP other.value.u8); \
    else if (type == Type::u16) return VMValue(value.u16 OP other.value.u16); \
    else if (type == Type::u32) return VMValue(value.u32 OP other.value.u32); \
    else if (type == Type::u64) return VMValue(value.u64 OP other.value.u64); \
    else if (type == Type::i8) return VMValue(value.i8 OP other.value.i8); \
    else if (type == Type::i16) return VMValue(value.i16 OP other.value.i16); \
    else if (type == Type::i32) return VMValue(value.i32 OP other.value.i32); \
    else if (type == Type::i64) return VMValue(value.i64 OP other.value.i64); \
    else if (type == Type::f32) return VMValue(value.f32 OP other.value.f32); \
    else if (type == Type::f64) return VMValue(value.f64 OP other.value.f64); \
    else if (type == Type::boolean) return VMValue(value.boolean OP other.value.boolean); \
    else return VMValue();


    VMValue::VMValue(): type(Type::null) {}
	VMValue::VMValue(uint8_t val) : type(Type::u8) { value.u8 = val; }
	VMValue::VMValue(uint16_t val) : type(Type::u16) { value.u16 = val; }
	VMValue::VMValue(uint32_t val) : type(Type::u32) { value.u32 = val; }
	VMValue::VMValue(uint64_t val) : type(Type::u64) { value.u64 = val; }
	VMValue::VMValue(int8_t val) : type(Type::i8) { value.i8 = val; }
	VMValue::VMValue(int16_t val) : type(Type::i16) { value.i16 = val; }
	VMValue::VMValue(int32_t val) : type(Type::i32) { value.i32 = val; }
	VMValue::VMValue(int64_t val) : type(Type::i64) { value.i64 = val; }
	VMValue::VMValue(float val) : type(Type::f32) { value.f32 = val; }
	VMValue::VMValue(double val) : type(Type::f64) { value.f64 = val; }
	VMValue::VMValue(bool val) : type(Type::boolean) { value.boolean = val; }
	VMValue::VMValue(const char* val) : type(Type::string) { value.string = val; }
	VMValue::VMValue(VMObject* val) : type(Type::object) { value.object = val; }

    VMValue VMValue::operator==(const VMValue& other) const {
        VMVALUE_OP(==);
    }

    VMValue VMValue::operator!=(const VMValue& other) const {
        VMVALUE_OP(!=);
    }

    VMValue VMValue::operator<(const VMValue& other) const {
        VMVALUE_OP(<);
    }

    VMValue VMValue::operator>(const VMValue& other) const {
        VMVALUE_OP(>);
    }

    VMValue VMValue::operator<=(const VMValue& other) const {
        VMVALUE_OP(<=);
    }

    VMValue VMValue::operator>=(const VMValue& other) const {
        VMVALUE_OP(>=);
    }

    VMValue VMValue::operator&&(const VMValue& other) const {
        VMVALUE_OP(&&);
    }

    VMValue VMValue::operator||(const VMValue& other) const {
        VMVALUE_OP(||);
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
		if (type == Type::u8) return value.u8 != 0;
		else if (type == Type::u16) return value.u16 != 0;
		else if (type == Type::u32) return value.u32 != 0;
		else if (type == Type::u64) return value.u64 != 0;
		else if (type == Type::i8) return value.i8 != 0;
		else if (type == Type::i16) return value.i16 != 0;
		else if (type == Type::i32) return value.i32 != 0;
		else if (type == Type::i64) return value.i64 != 0;
		else if (type == Type::f32) return value.f32 != 0;
		else if (type == Type::f64) return value.f64 != 0;
		else if (type == Type::boolean) return value.boolean;
        else if (type == Type::null) return false;
        else if (type == Type::string) return true;
        else if (type == Type::object) return value.object != nullptr;
		else return false;
	}

    bool VMValue::equals(const VMValue& other) const {
        if (type != other.type) return false;

		if (type == Type::u8) return value.u8 == other.value.u8;
		else if (type == Type::u16) return value.u16 == other.value.u16;
		else if (type == Type::u32) return value.u32 == other.value.u32;
		else if (type == Type::u64) return value.u64 == other.value.u64;
		else if (type == Type::i8) return value.i8 == other.value.i8;
		else if (type == Type::i16) return value.i16 == other.value.i16;
		else if (type == Type::i32) return value.i32 == other.value.i32;
		else if (type == Type::i64) return value.i64 == other.value.i64;
		else if (type == Type::f32) return value.f32 == other.value.f32;
		else if (type == Type::f64) return value.f64 == other.value.f64;
		else if (type == Type::boolean) return value.boolean == other.value.boolean;
        else if (type == Type::null) return true;
        else if (type == Type::string) return value.string == other.value.string;
        else if (type == Type::object) return value.object == other.value.object;
		else return false;
    }

    std::ostream& operator<<(std::ostream& str, const VMValue& v) {
        if (v.type == VMValue::Type::u8) {
            str.write("1", 1);
            str.write((const char*)&v.value.u8, 1);
        }
        else if (v.type == VMValue::Type::u16) {
            str.write("2", 1);
            str.write((const char*)&v.value.u16, 2);
        }
        else if (v.type == VMValue::Type::u32) {
            str.write("3", 1);
            str.write((const char*)&v.value.u32, 4);
        }
        else if (v.type == VMValue::Type::u64) {
            str.write("4", 1);
            str.write((const char*)&v.value.u64, 8);
        }
        else if (v.type == VMValue::Type::i8) {
            str.write("5", 1);
            str.write((const char*)&v.value.i8, 1);
        }
        else if (v.type == VMValue::Type::i16) {
            str.write("6", 1);
            str.write((const char*)&v.value.i16, 2);
        }
        else if (v.type == VMValue::Type::i32) {
            str.write("7", 1);
            str.write((const char*)&v.value.i32, 4);
        }
        else if (v.type == VMValue::Type::i64) {
            str.write("8", 1);
            str.write((const char*)&v.value.i64, 8);
        }
        else if (v.type == VMValue::Type::f32) {
            str.write("f", 1);
            str.write((const char*)&v.value.f32, 4);
        }
        else if (v.type == VMValue::Type::f64) {
            str.write("d", 1);
            str.write((const char*)&v.value.f64, 8);
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

        if (t == '1') {
            v.type = VMValue::Type::u8;
            str.read((char*)&v.value.u8, 1);
        }
        else if (t == '2') {
            v.type = VMValue::Type::u16;
            str.read((char*)&v.value.u16, 2);
        }
        else if (t == '3') {
            v.type = VMValue::Type::u32;
            str.read((char*)&v.value.u32, 4);
        }
        else if (t == '4') {
            v.type = VMValue::Type::u64;
            str.read((char*)&v.value.u64, 8);
        }
        else if (t == '5') {
            v.type = VMValue::Type::i8;
            str.read((char*)&v.value.i8, 1);
        }
        else if (t == '6') {
            v.type = VMValue::Type::i16;
            str.read((char*)&v.value.i16, 2);
        }
        else if (t == '7') {
            v.type = VMValue::Type::i32;
            str.read((char*)&v.value.i32, 4);
        }
        else if (t == '8') {
            v.type = VMValue::Type::i64;
            str.read((char*)&v.value.i64, 8);
        }
        else if (t == 'f') {
            v.type = VMValue::Type::f32;
            str.read((char*)&v.value.f32, 4);
        }
        else if (t == 'd') {
            v.type = VMValue::Type::f64;
            str.read((char*)&v.value.f64, 8);
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