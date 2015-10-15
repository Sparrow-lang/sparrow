
#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/Alloc.h"

StringRef allocStringRef(unsigned size) {
    StringRef res = { 0, 0 };
    if ( size == 0 )
        return res;
    char* p = (char*) alloc(size+1, allocString); // extra byte for null termination;
    res.begin = p;
    res.end = res.begin + size;
    p[size] = 0;
    return res;
}

StringRef fromCStr(const char* str) {
    StringRef res = { str, str+strlen(str) };
    return res;
}
StringRef fromString(const string& str) {
    StringRef res = { &str[0], (&str[0])+str.size() };
    return res;

}

string toString(StringRef str) {
    return string(str.begin, str.end);
}

StringRef dupCStr(const char* str) {
    unsigned size = strlen(str);
    StringRef res = allocStringRef(size);
    if ( size > 0 )
        memcpy((char*) res.begin, str, size);
    return res;
}
StringRef dupString(const string& str) {
    unsigned size = str.size();
    StringRef res = allocStringRef(size);
    if ( size > 0 )
        memcpy((char*) res.begin, str.c_str(), size);
    return res;
}

StringRef dup(StringRef str) {
    unsigned sz = size(str);
    StringRef res = allocStringRef(sz);
    if ( sz > 0 )
        memcpy((char*) res.begin, str.begin, sz);
    return res;
}

unsigned size(StringRef str) {
    return str.end - str.begin;
}

bool operator == (StringRef lhs, StringRef rhs) {
    if ( lhs.end-lhs.begin != rhs.end-rhs.begin )
        return false;
    const char* p1 = lhs.begin;
    const char* p2 = rhs.begin;
    for ( ; p1!=lhs.end; ++p1, ++p2 )
        if ( *p1 != *p2 )
            return false;
    return true;
}

bool operator != (StringRef lhs, StringRef rhs) {
    return !(lhs == rhs);
}


bool operator == (StringRef lhs, const char* rhs) {
    return lhs == fromCStr(rhs);
}

bool operator != (StringRef lhs, const char* rhs) {
    return !(lhs == fromCStr(rhs));
}

ostream& operator << (ostream& os, StringRef str)
{
    if ( size(str) > 0 )
        os << toString(str);
    return os;
}
