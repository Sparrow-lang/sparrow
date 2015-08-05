#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_STRUCT1(Nest, Node);

namespace Feather
{
    /// Getter for the value type of a CtValue node
    Nest::TypeRef getCtValueType(Nest::Node* ctVal);

    /// Getter for the value data of a CtValue node -- the data is encoded in a string
    const string& getCtValueDataAsString(Nest::Node* ctVal);

    /// Getter for the value memory buffer of this value
    template <typename T>
    T* getCtValueData(Nest::Node* ctVal)
    {
        return (T*) (void*) getCtValueDataAsString(ctVal).c_str();
    }

    bool getBoolCtValue(Nest::Node* ctVal);

    /// Check if the two given CtValue objects are equal -- contains the same type and data
    bool sameCtValue(Nest::Node* ctVal1, Nest::Node* ctVal2);
}
