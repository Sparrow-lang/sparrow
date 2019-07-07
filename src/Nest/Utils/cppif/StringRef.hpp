#pragma once

#include "Nest/Api/StringRef.h"
#include <string>

namespace Nest {

/**
 * @brief   A reference to a string.
 *
 * This does not hold ownership of the given string.
 * It does not need zero termination, so therefore it can hold zeros inside it. It also can
 * reference substrings.
 *
 * Easily convertible to and from const char*, std::string, Nest_StringRef.
 *
 * @see     Nest_StringRef.
 */
struct StringRef : Nest_StringRef {
    StringRef()
        : Nest_StringRef{nullptr, nullptr} {}
    StringRef(const char* b, const char* e)
        : Nest_StringRef{b, e} {}
    StringRef(Nest_StringRef str)
        : Nest_StringRef{str} {}
    StringRef(const char* str)
        : Nest_StringRef{str, str + strlen(str)} {}
    StringRef(const string& str);

    //! Duplicates this string. Allocates memory with the special string bulk allocator.
    StringRef dup() const;

    //! Converts this into a std::string
    string toStd() const { return begin ? string(begin, end) : string(); }

    //! Checks if this string is empty
    bool empty() const { return end == begin; }

    //! Checks if the string is not empty
    explicit operator bool() const { return end != begin; }

    //! Returns the number of chars (bytes) in this string.
    int size() const;
};

//! The mutable version of StringRef -- we can modify its content
struct StringRefM {
    char* begin{nullptr}; ///< The beginning of the string
    char* end{nullptr};   ///< One past the last character of the string

    StringRefM() {}
    StringRefM(int size);

    operator StringRef() { return {begin, end}; }
    operator Nest_StringRef() { return {begin, end}; }

    //! Checks if this string is empty
    bool empty() const { return end == begin; }

    //! Checks if the string is not empty
    explicit operator bool() const { return end != begin; }

    //! Returns the number of chars (bytes) in this string.
    int size() const;
};

//! Equality/inequality check over StringRef objects
bool operator==(StringRef lhs, StringRef rhs);
bool operator!=(StringRef lhs, StringRef rhs);

//! Stream dump operator
ostream& operator<<(ostream& os, StringRef str);

} // namespace Nest

ostream& operator<<(ostream& os, Nest_StringRef str);

namespace std {

template <> struct hash<Nest::StringRef> {
    using argument_type = Nest::StringRef;
    size_t operator()(const Nest::StringRef& s) const noexcept {
        size_t seed = 0;
        for (const char* p = s.begin; p != s.end; p++)
            seed ^= size_t(*p) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <> struct hash<Nest_StringRef> {
    using argument_type = Nest_StringRef;
    size_t operator()(const Nest_StringRef& s) const noexcept { return hash<Nest::StringRef>()(s); }
};

} // namespace std
