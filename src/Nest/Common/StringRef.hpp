#pragma once

#include "StringRef.h"
#include <string>

StringRef allocStringRef(unsigned size);

StringRef fromCStr(const char* str);
StringRef fromString(const string& str);

string toString(StringRef str);

StringRef dupCStr(const char* str);
StringRef dupString(const string& str);

StringRef dup(StringRef str);

unsigned size(StringRef str);

bool operator == (StringRef lhs, StringRef rhs);
bool operator != (StringRef lhs, StringRef rhs);

bool operator == (StringRef lhs, const char* rhs);
bool operator != (StringRef lhs, const char* rhs);

ostream& operator << (ostream& os, StringRef str);
