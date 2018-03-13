#pragma once

#include "Nest/Api/StringRef.h"
#include <string>

StringRef allocStringRef(unsigned size);

StringRef fromCStr(const char* str);
StringRef fromString(const string& str);

string toString(StringRef str);

StringRef dupCStr(const char* str);
StringRef dupString(const string& str);

StringRef dup(StringRef str);

unsigned size(StringRef str);

bool operator==(StringRef lhs, StringRef rhs);
bool operator!=(StringRef lhs, StringRef rhs);

bool operator==(StringRef lhs, const char* rhs);
bool operator!=(StringRef lhs, const char* rhs);

ostream& operator<<(ostream& os, StringRef str);

namespace std {

template <> struct hash<StringRef> {
    using argument_type = StringRef;
    size_t operator()(const StringRef& s) const noexcept {
        size_t seed = 0;
        for (const char* p = s.begin; p != s.end; p++)
            seed ^= size_t(*p) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

} // namespace std
