// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "GC.h"
#include "VMType.h"

#include <memory.h>

namespace Strela {
    VMObject* GC::allocObject(const VMType* type) {
        auto obj = (VMObject*)calloc(1, sizeof(VMObject) + type->objectSize);
        obj->type = type;
        objects.push_back(obj);
        return obj;
    }

    VMObject* GC::allocArray(const VMType* type, uint64_t length) {
        auto obj = (VMObject*)calloc(1, sizeof(VMObject) + sizeof(uint64_t) + type->arrayType->size * length);
        obj->type = type;
        memcpy(obj->data, &length, sizeof(length));
        objects.push_back(obj);
        return obj;
    }

    void GC::collect(std::vector<VMValue>& stack) {
        if (objects.empty()) return;

        for (auto&& val: stack) {
            if (val.type == VMValue::Type::object) {
                mark(val.value.object);
            }
        }
        
        auto it = objects.begin();
        while (it != objects.end()) {
            auto obj = *it;
            if (obj->marked) {
                obj->marked = false;
                ++it;
            }
            else {
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
                    mark(*(VMObject**)ptr);
                    ptr += sizeof(VMObject*);
                }
            }
        }
        else {
            for (auto&& field: type->fields) {
                if (field.type == nullptr || !(field.type->isArray || field.type->isObject)) continue;
                mark(*(VMObject**)&object->data[field.offset]);
            }
        }
    }
}