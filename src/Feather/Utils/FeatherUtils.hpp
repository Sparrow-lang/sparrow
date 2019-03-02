#pragma once

#include "Feather/Utils/FeatherUtils.h"

#include "Nest/Api/EvalMode.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Api/Location.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/Type.hpp"

using Node = struct Nest_Node;
using CompilationContext = struct Nest_CompilationContext;

using Nest::Location;
using Nest::StringRef;
using Nest::Type;

/// Getter for the value memory buffer of this value
template <typename T> T* Feather_getCtValueData(Node* ctVal) {
    return (T*)(void*)Nest_getCheckPropertyString(ctVal, "valueData").begin;
}

template <typename T> Node* Feather_mkCtValueT(const Location& loc, Type type, T* dataVal) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* p = reinterpret_cast<const char*>(dataVal);
    StringRef dataStr = {p, p + sizeof(*dataVal)};
    return Feather_mkCtValue(loc, type, dataStr);
}
