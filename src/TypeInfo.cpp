// Copyright (c) 2018 Stephan Unverwerth
// This code is licensed under MIT license (See LICENSE for details)

#include "TypeInfo.h"

namespace Strela {
	
	bool TypeInfo::extends(const TypeInfo* tid) const {
		if (tid == this) return false;
		if (!parent) return false;
		if (tid == parent) return true;

		auto it = extCache.find(tid);
		if (it != extCache.end()) return it->second;

		bool ext = parent->extends(tid);
		extCache.insert(std::make_pair(tid, ext));
		return ext;
	}
}