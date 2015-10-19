#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"

typedef struct Nest_Node Node;

namespace Feather
{
    /// Getter for the value type of a CtValue node
    TypeRef getCtValueType(Node* ctVal);

    /// Getter for the value data of a CtValue node -- the data is encoded in a string
    StringRef getCtValueDataAsString(Node* ctVal);

    /// Getter for the value memory buffer of this value
    template <typename T>
    T* getCtValueData(Node* ctVal)
    {
        return (T*) (void*) getCtValueDataAsString(ctVal).begin;
    }

    bool getBoolCtValue(Node* ctVal);

    /// Check if the two given CtValue objects are equal -- contains the same type and data
    bool sameCtValue(Node* ctVal1, Node* ctVal2);
}
