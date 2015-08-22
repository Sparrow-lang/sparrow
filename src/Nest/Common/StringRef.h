#pragma once

/// A reference to a string, or, range of characters
struct Nest_StringRef {
    char* begin;    ///< The beginning of the string
    char* end;      ///< One past the last character of the string
};
typedef struct Nest_StringRef Nest_StringRef;
typedef struct Nest_StringRef StringRef;

