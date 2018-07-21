
#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/Alloc.h"

namespace Nest {
StringRef::StringRef(const string& str)
    : Nest_StringRef{nullptr, nullptr} {
    if (!str.empty()) {
        begin = &str[0];
        end = &str[0] + str.size();
    }
}

StringRef StringRef::dup() const {
    auto sz = size();
    StringRefM res{sz};
    if (sz > 0)
        memcpy(res.begin, begin, sz);
    return StringRef{res.begin, res.end};
}

int StringRef::size() const {
    ASSERT(end >= begin);
    return end - begin;
}

StringRefM::StringRefM(int size)
    : begin(nullptr)
    , end(nullptr) {
    if (size > 0) {
        auto* p = allocStr(size + 1); // extra byte for null termination;
        begin = p;
        end = begin + size;
        p[size] = 0;
    }
}

int StringRefM::size() const {
    ASSERT(end >= begin);
    return end - begin;
}

bool operator==(StringRef lhs, StringRef rhs) {
    if (lhs.end - lhs.begin != rhs.end - rhs.begin)
        return false;
    const char* p1 = lhs.begin;
    const char* p2 = rhs.begin;
    for (; p1 != lhs.end; ++p1, ++p2)
        if (*p1 != *p2)
            return false;
    return true;
}

bool operator!=(StringRef lhs, StringRef rhs) { return !(lhs == rhs); }

ostream& operator<<(ostream& os, StringRef str) {
    if (str.size() > 0)
        os << str.toStd();
    return os;
}

} // namespace Nest

ostream& operator<<(ostream& os, Nest_StringRef str) { return os << Nest::StringRef(str); }

// Nest_StringRef fromCStr(const char* str) {
//     Nest_StringRef res = {str, str + strlen(str)};
//     return res;
// }
// Nest_StringRef fromString(const string& str) {
//     if (str.empty())
//         return Nest_StringRef{nullptr, nullptr};
//     Nest_StringRef res = {&str[0], (&str[0]) + str.size()};
//     return res;
// }

// string toString(Nest_StringRef str) { return str.begin ? string(str.begin, str.end) : string(); }

// Nest_StringRef dupCStr(const char* str) {
//     int size = strlen(str);
//     Nest::StringRefM res{size};
//     if (size > 0)
//         memcpy(res.begin, str, size);
//     return res;
// }
// Nest_StringRef dupString(const string& str) {
//     int size = str.size();
//     Nest::StringRefM res{size};
//     if (size > 0)
//         memcpy(res.begin, str.c_str(), size);
//     return res;
// }

// Nest_StringRef dup(Nest_StringRef str) {
//     int sz = size(str);
//     Nest::StringRefM res{sz};
//     if (sz > 0)
//         memcpy(res.begin, str.begin, sz);
//     return res;
// }

// unsigned size(Nest_StringRef str) {
//     ASSERT(str.end >= str.begin);
//     return str.end - str.begin;
// }

// bool operator==(Nest_StringRef lhs, Nest_StringRef rhs) {
//     if (lhs.end - lhs.begin != rhs.end - rhs.begin)
//         return false;
//     const char* p1 = lhs.begin;
//     const char* p2 = rhs.begin;
//     for (; p1 != lhs.end; ++p1, ++p2)
//         if (*p1 != *p2)
//             return false;
//     return true;
// }

// bool operator!=(Nest_StringRef lhs, Nest_StringRef rhs) { return !(lhs == rhs); }

// bool operator==(Nest_StringRef lhs, const char* rhs) { return lhs == fromCStr(rhs); }

// bool operator!=(Nest_StringRef lhs, const char* rhs) { return !(lhs == fromCStr(rhs)); }
