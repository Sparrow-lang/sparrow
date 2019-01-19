#pragma once

#include "Common/TypeFactory.hpp"

using Feather::TypeWithStorage;

//! Builds a string from a string prefix and an integer
inline std::string concat(const char* prefix, int num) {
    ostringstream oss;
    oss << prefix << num;
    return oss.str();
}

//! Generates a random boolean, given its 'true' probability
inline bool randomChance(int percentage) { return *rc::gen::inRange(0, 100) < percentage; }
