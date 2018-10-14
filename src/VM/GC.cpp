// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "GC.h"
#include "VMType.h"

#include <memory.h>

namespace Strela {
    void* GC::allocObject(const VMType* type) {
        auto obj = (VMObject*)calloc(sizeof(VMObject) + type->objectSize, 1);
        obj->type = type;
        objects.push_back(obj);
        //std::cout << "Alloc object " << type->name << "\n";
        return obj + 1;
    }

    void* GC::allocArray(const VMType* type, uint64_t length) {
        auto obj = (VMObject*)calloc(sizeof(VMObject) + sizeof(uint64_t) + type->arrayType->size * length, 1);
        obj->type = type;
        memcpy(obj->data, &length, sizeof(length));
        objects.push_back(obj);
        //std::cout << "Alloc array " << type->name << "\n";
        return obj + 1;
    }

    void GC::collect(std::vector<VMValue>& stack) {
        if (objects.empty()) return;

        for (auto&& val: stack) {
            if (val.type == VMValue::Type::object && val.value.object) {
                mark((VMObject*)val.value.object - 1);
            }
        }
        
        for (auto&& obj: lockList) {
            mark(obj);
        }
        
        auto it = objects.begin();
        while (it != objects.end()) {
            auto obj = *it;
            if (obj->marked) {
                obj->marked = false;
                ++it;
            }
            else {
                //std::cout << "Free " << obj->type->name << "\n";
                free(obj);
                it = objects.erase(it);
            }
        }
    }

    void GC::mark(VMObject* object) {
        if (object == nullptr) return;
        if (object->marked) return;
        if (object->type == nullptr) return;

        const auto& type = object->type;
        
        object->marked = true;

        if (type->isArray) {
            if (type->arrayType->isArray || type->arrayType->isObject) {
                char* ptr = object->data;
                uint64_t length;
                memcpy(&length, ptr, 8);
                ptr += 8;
                while (length--) {
                    if (*(void**)ptr) {
                        mark(*(VMObject**)ptr - 1);
                    }
                    ptr += 8;
                }
            }
        }
        else if (type->unionTypes.size()) {
            uint64_t tag = *(uint64_t*)object->data;
            void* ref = *(void**)&object->data[8];
            VMType* refType = type->unionTypes[tag];
            if (ref && refType->isObject) {
                mark((VMObject*)ref - 1);
            }
        }
        else {
            for (auto&& field: type->fields) {
                if (field.type == nullptr || !(field.type->isArray || field.type->isObject)) continue;
                if (*(void**)&object->data[field.offset]) {
                    mark(*(VMObject**)&object->data[field.offset] - 1);
                }
            }
        }
    }

    
    void GC::lock(void* obj) {
        lockList.insert((VMObject*)obj - 1);
    }

    void GC::unlock(void* obj) {
        lockList.erase((VMObject*)obj - 1);
    }
}