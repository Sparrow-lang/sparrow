#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_CLASS1(Feather, Class);

namespace Feather
{
    using namespace Nest;

    /// Returns the Void type corresponding to the given evaluation mode
    TypeRef getVoidType(EvalMode mode);

    /// Returns a type that represents data
    /// A data type is introduced by a class definition and can have one or more references; a data type must have a size
    TypeRef getDataType(Class* classDecl, uint8_t numReferences = 0, EvalMode mode = modeRtCt);

    /// Returns an L-Value type
    /// This type would represents an l-value to an existing storage type
    TypeRef getLValueType(TypeRef base);

    /// Returns a type that holds N instances of a given storage type
    TypeRef getArrayType(TypeRef unitType, uint32_t count);

    /// Returns a type that represents a classic function, with parameters and result type
    /// This type can be constructed from a set of parameter types and a result type
    TypeRef getFunctionType(TypeRef resultType, const vector<TypeRef>& paramTypes, EvalMode mode);
}
