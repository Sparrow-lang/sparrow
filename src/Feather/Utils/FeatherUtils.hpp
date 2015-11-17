#pragma once

#include "Feather/Utils/FeatherUtils.h"

#include "Nest/Api/EvalMode.h"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Api/Location.h"
#include "Nest/Utils/NodeUtils.h"

typedef struct Nest_Node Node;
typedef struct Nest_CompilationContext CompilationContext;

/// Getter for the value memory buffer of this value
template <typename T>
T* Feather_getCtValueData(Node* ctVal)
{
    return (T*) (void*) Nest_getCheckPropertyString(ctVal, "valueData").begin;
}

template <typename T>
Node* Feather_mkCtValueT(const Location& loc, TypeRef type, T* dataVal)
{
    const char* p = reinterpret_cast<const char*>(dataVal);
    StringRef dataStr = {p, p+sizeof(*dataVal)};
    return Feather_mkCtValue(loc, type, dataStr);
}


