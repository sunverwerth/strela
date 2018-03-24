// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "GC.h"

namespace Strela {
    VMObject* GC::allocObject(size_t numFields) {
        auto obj = new VMObject(numFields);
        objects.push_back(obj);
        return obj;
    }

    void GC::collect(const std::vector<VMValue>& stack) {
        if (objects.empty()) return;
        
        for (auto&& val: stack) {
            if (val.type == VMValue::Type::object && val.value.object && !(val.value.object->flags & 1)) {
                mark(val.value.object);
            }
        }

        for (auto it = objects.begin(); it != objects.end(); ++it) {
            auto obj = *it;
            if (!(obj->flags & 1)) {
                delete obj;
                it = objects.erase(it);
            }
            else {
                obj->flags &= ~1;
            }
        }
    }

    void GC::mark(VMObject* object) {
        object->flags |= 1;
        
        for (auto& field: object->fields) {
            if (field.type == VMValue::Type::object && field.value.object && !(field.value.object->flags & 1)) {
                mark(field.value.object);
            }
        }
    }
}